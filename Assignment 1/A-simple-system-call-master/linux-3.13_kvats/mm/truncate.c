/*
 * mm/truncate.c - code for taking down pages from address_spaces
 *
 * Copyright (C) 2002, Linus Torvalds
 *
 * 10Sep2002	Andrew Morton
 *		Initial version.
 */

#include <linux/kernel.h>
#include <linux/backing-dev.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/export.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/pagevec.h>
#include <linux/task_io_accounting_ops.h>
#include <linux/buffer_head.h>	/* grr. try_to_release_page,
				   do_invalidatepage */
#include <linux/cleancache.h>
#include "internal.h"


/**
 * do_invalidatepage - invalidate part or all of a page
 * @page: the page which is affected
 * @offset: start of the range to invalidate
 * @length: length of the range to invalidate
 *
 * do_invalidatepage() is called when all or part of the page has become
 * invalidated by a truncate operation.
 *
 * do_invalidatepage() does not have to release all buffers, but it must
 * ensure that no dirty buffer is left outside @offset and that no I/O
 * is underway against any of the blocks which are outside the truncation
 * point.  Because the caller is about to free (and possibly reuse) those
 * blocks on-disk.
 */
void do_invalidatepage(struct page *page, unsigned int offset,
		       unsigned int length)
{
	void (*invalidatepage)(struct page *, unsigned int, unsigned int);

	invalidatepage = page->mapping->a_ops->invalidatepage;
#ifdef CONFIG_BLOCK
	if (!invalidatepage)
		invalidatepage = block_invalidatepage;
#endif
	if (invalidatepage)
		(*invalidatepage)(page, offset, length);
}

/*
 * This cancels just the dirty bit on the kernel page itself, it
 * does NOT actually remove dirty bits on any mmap's that may be
 * around. It also leaves the page tagged dirty, so any sync
 * activity will still find it on the dirty lists, and in particular,
 * clear_page_dirty_for_io() will still look at the dirty bits in
 * the VM.
 *
 * Doing this should *normally* only ever be done when a page
 * is truncated, and is not actually mapped anywhere at all. However,
 * fs/buffer.c does this when it notices that somebody has cleaned
 * out all the buffers on a page without actually doing it through
 * the VM. Can you say "ext3 is horribly ugly"? Tought you could.
 */
void cancel_dirty_page(struct page *page, unsigned int account_size)
{
	if (TestClearPageDirty(page)) {
		struct address_space *mapping = page->mapping;
		if (mapping && mapping_cap_account_dirty(mapping)) {
			dec_zone_page_state(page, NR_FILE_DIRTY);
			dec_bdi_stat(mapping->backing_dev_info,
					BDI_RECLAIMABLE);
			if (account_size)
				task_io_account_cancelled_write(account_size);
		}
	}
}
EXPORT_SYMBOL(cancel_dirty_page);

/*
 * If truncate cannot remove the fs-private metadata from the page, the page
 * becomes orphaned.  It will be left on the LRU and may even be mapped into
 * user pagetables if we're racing with filemap_fault().
 *
 * We need to bale out if page->mapping is no longer equal to the original
 * mapping.  This happens a) when the VM reclaimed the page while we waited on
 * its lock, b) when a concurrent invalidate_mapping_pages got there first and
 * c) when tmpfs swizzles a page between a tmpfs inode and swapper_space.
 */
static int
truncate_complete_page(struct address_space *mapping, struct page *page)
{
	if (page->mapping != mapping)
		return -EIO;

	if (page_has_private(page))
		do_invalidatepage(page, 0, PAGE_CACHE_SIZE);

	cancel_dirty_page(page, PAGE_CACHE_SIZE);

	ClearPageMappedToDisk(page);
	delete_from_page_cache(page);
	return 0;
}

/*
 * This is for invalidate_mapping_pages().  That function can be called at
 * any time, and is not supposed to throw away dirty pages.  But pages can
 * be marked dirty at any time too, so use remove_mapping which safely
 * discards clean, unused pages.
 *
 * Returns non-zero if the page was successfully invalidated.
 */
static int
invalidate_complete_page(struct address_space *mapping, struct page *page)
{
	int ret;

	if (page->mapping != mapping)
		return 0;

	if (page_has_private(page) && !try_to_release_page(page, 0))
		return 0;

	ret = remove_mapping(mapping, page);

	return ret;
}

int truncate_inode_page(struct address_space *mapping, struct page *page)
{
	if (page_mapped(page)) {
		unmap_mapping_range(mapping,
				   (loff_t)page->index << PAGE_CACHE_SHIFT,
				   PAGE_CACHE_SIZE, 0);
	}
	return truncate_complete_page(mapping, page);
}

/*
 * Used to get rid of pages on hardware memory corruption.
 */
int generic_error_remove_page(struct address_space *mapping, struct page *page)
{
	if (!mapping)
		return -EINVAL;
	/*
	 * Only punch for normal data pages for now.
	 * Handling other types like directories would need more auditing.
	 */
	if (!S_ISREG(mapping->host->i_mode))
		return -EIO;
	return truncate_inode_page(mapping, page);
}
EXPORT_SYMBOL(generic_error_remove_page);

/*
 * Safely invalidate one page from its pagecache mapping.
 * It only drops clean, unused pages. The page must be locked.
 *
 * Returns 1 if the page is successfully invalidated, otherwise 0.
 */
int invalidate_inode_page(struct page *page)
{
	struct address_space *mapping = page_mapping(page);
	if (!mapping)
		return 0;
	if (PageDirty(page) || PageWriteback(page))
		return 0;
	if (page_mapped(page))
		return 0;
	return invalidate_complete_page(mapping, page);
}

/**
 * truncate_inode_pages_range - truncate range of pages specified by start & end byte offsets
 * @mapping: mapping to truncate
 * @lstart: offset from which to truncate
 * @lend: offset to which to truncate (inclusive)
 *
 * Truncate the page cache, removing the pages that are between
 * specified offsets (and zeroing out partial pages
 * if lstart or lend + 1 is not page aligned).
 *
 * Truncate takes two passes - the first pass is nonblocking.  It will not
 * block on page locks and it will not block on writeback.  The second pass
 * will wait.  This is to prevent as much IO as possible in the affected region.
 * The first pass will remove most pages, so the search cost of the second pass
 * is low.
 *
 * We pass down the cache-hot hint to the page freeing code.  Even if the
 * mapping is large, it is probably the case that the final pages are the most
 * recently touched, and freeing happens in ascending file offset order.
 *
 * Note that since ->invalidatepage() accepts range to invalidate
 * truncate_inode_pages_range is able to handle cases where lend + 1 is not
 * page aligned properly.
 */
void truncate_inode_pages_range(struct address_space *mapping,
				loff_t lstart, loff_t lend)
{
	pgoff_t		start;		/* inclusive */
	pgoff_t		end;		/* exclusive */
	unsigned int	partial_start;	/* inclusive */
	unsigned int	partial_end;	/* exclusive */
	struct pagevec	pvec;
	pgoff_t		index;
	int		i;

	cleancache_invalidate_inode(mapping);
	if (mapping->nrpages == 0)
		return;

	/* Offsets within partial pages */
	partial_start = lstart & (PAGE_CACHE_SIZE - 1);
	partial_end = (lend + 1) & (PAGE_CACHE_SIZE - 1);

	/*
	 * 'start' and 'end' always covers the range of pages to be fully
	 * truncated. Partial pages are covered with 'partial_start' at the
	 * start of the range and 'partial_end' at the end of the range.
	 * Note that 'end' is exclusive while 'lend' is inclusive.
	 */
	start = (lstart + PAGE_CACHE_SIZE - 1) >> PAGE_CACHE_SHIFT;
	if (lend == -1)
		/*
		 * lend == -1 indicates end-of-file so we have to set 'end'
		 * to the highest possible pgoff_t and since the type is
		 * unsigned we're using -1.
		 */
		end = -1;
	else
		end = (lend + 1) >> PAGE_CACHE_SHIFT;

	pagevec_init(&pvec, 0);
	index = start;
	while (index < end && pagevec_lookup(&pvec, mapping, index,
			min(end - index, (pgoff_t)PAGEVEC_SIZE))) {
		mem_cgroup_uncharge_start();
		for (i = 0; i < pagevec_count(&pvec); i++) {
			struct page *page = pvec.pages[i];

			/* We rely upon deletion not changing page->index */
			index = page->index;
			if (index >= end)
				break;

			if (!trylock_page(page))
				continue;
			WARN_ON(page->index != index);
			if (PageWriteback(page)) {
				unlock_page(page);
				continue;
			}
			truncate_inode_page(mapping, page);
			unlock_page(page);
		}
		pagevec_release(&pvec);
		mem_cgroup_uncharge_end();
		cond_resched();
		index++;
	}

	if (partial_start) {
		struct page *page = find_lock_page(mapping, start - 1);
		if (page) {
			unsigned int top = PAGE_CACHE_SIZE;
			if (start > end) {
				/* Truncation within a single page */
				top = partial_end;
				partial_end = 0;
			}
			wait_on_page_writeback(page);
			zero_user_segment(page, partial_start, top);
			cleancache_invalidate_page(mapping, page);
			if (page_has_private(page))
				do_invalidatepage(page, partial_start,
						  top - partial_start);
			unlock_page(page);
			page_cache_release(page);
		}
	}
	if (partial_end) {
		struct page *page = find_lock_page(mapping, end);
		if (page) {
			wait_on_page_writeback(page);
			zero_user_segment(page, 0, partial_end);
			cleancache_invalidate_page(mapping, page);
			if (page_has_private(page))
				do_invalidatepage(page, 0,
						  partial_end);
			unlock_page(page);
			page_cache_release(page);
		}
	}
	/*
	 * If the truncation happened within a single page no pages
	 * will be released, just zeroed, so we can bail out now.
	 */
	if (start >= end)
		return;

	index = start;
	for ( ; ; ) {
		cond_resched();
		if (!pagevec_lookup(&pvec, mapping, index,
			min(end - index, (pgoff_t)PAGEVEC_SIZE))) {
			if (index == start)
				break;
			index = start;
			continue;
		}
		if (index == start && pvec.pages[0]->index >= end) {
			pagevec_release(&pvec);
			break;
		}
		mem_cgroup_uncharge_start();
		for (i = 0; i < pagevec_count(&pvec); i++) {
			struct page *page = pvec.pages[i];

			/* We rely upon deletion not changing page->index */
			index = page->index;
			if (index >= end)
				break;

			lock_page(page);
			WARN_ON(page->index != index);
			wait_on_page_writeback(page);
			truncate_inode_page(mapping, page);
			unlock_page(page);
		}
		pagevec_release(&pvec);
		mem_cgroup_uncharge_end();
		index++;
	}
	cleancache_invalidate_inode(mapping);
}
EXPORT_SYMBOL(truncate_inode_pages_range);

/**
 * truncate_inode_pages - truncate *all* the pages from an offset
 * @mapping: mapping to truncate
 * @lstart: offset from which to truncate
 *
 * Called under (and serialised by) inode->i_mutex.
 *
 * Note: When this function returns, there can be a page in the process of
 * deletion (inside __delete_from_page_cache()) in the specified range.  Thus
 * mapping->nrpages can be non-zero when this function returns even after
 * truncation of the whole mapping.
 */
void truncate_inode_pages(struct address_space *mapping, loff_t lstart)
{
	truncate_inode_pages_range(mapping, lstart, (loff_t)-1);
}
EXPORT_SYMBOL(truncate_inode_pages);

/**
 * invalidate_mapping_pages - Invalidate all the unlocked pages of one inode
 * @mapping: the address_space which holds the pages to invalidate
 * @start: the offset 'from' which to invalidate
 * @end: the offset 'to' which to invalidate (inclusive)
 *
 * This function only removes the unlocked pages, if you want to
 * remove all the pages of one inode, you must call truncate_inode_pages.
 *
 * invalidate_mapping_pages() will not block on IO activity. It will not
 * invalidate pages which are dirty, locked, under writeback or mapped into
 * pagetables.
 */
unsigned long invalidate_mapping_pages(struct address_space *mapping,
		pgoff_t start, pgoff_t end)
{
	struct pagevec pvec;
	pgoff_t index = start;
	unsigned long ret;
	unsigned long count = 0;
	int i;

	/*
	 * Note: this function may get called on a shmem/tmpfs mapping:
	 * pagevec_lookup() might then return 0 prematurely (because it
	 * got a gangful of swap entries); but it's hardly worth worrying
	 * about - it can rarely have anything to free from such a mapping
	 * (most pages are dirty), and already skips over any difficulties.
	 */

	pagevec_init(&pvec, 0);
	while (index <= end && pagevec_lookup(&pvec, mapping, index,
			min(end - index, (pgoff_t)PAGEVEC_SIZE - 1) + 1)) {
		mem_cgroup_uncharge_start();
		for (i = 0; i < pagevec_count(&pvec); i++) {
			struct page *page = pvec.pages[i];

			/* We rely upon deletion not changing page->index */
			index = page->index;
			if (index > end)
				break;

			if (!trylock_page(page))
				continue;
			WARN_ON(page->index != index);
			ret = invalidate_inode_page(page);
			unlock_page(page);
			/*
			 * Invalidation is a hint that the page is no longer
			 * of interest and try to speed up its reclaim.
			 */
			if (!ret)
				deactivate_page(page);
			count += ret;
		}
		pagevec_release(&pvec);
		mem_cgroup_uncharge_end();
		cond_resched();
		index++;
	}
	return count;
}
EXPORT_SYMBOL(invalidate_mapping_pages);

/*
 * This is like invalidate_complete_page(), except it ignores the page's
 * refcount.  We do this because invalidate_inode_pages2() needs stronger
 * invalidation guarantees, and cannot afford to leave pages behind because
 * shrink_page_list() has a temp ref on them, or because they're transiently
 * sitting in the lru_cache_add() pagevecs.
 */
static int
invalidate_complete_page2(struct address_space *mapping, struct page *page)
{
	if (page->mapping != mapping)
		return 0;

	if (page_has_private(page) && !try_to_release_page(page, GFP_KERNEL))
		return 0;

	spin_lock_irq(&mapping->tree_lock);
	if (PageDirty(page))
		goto failed;

	BUG_ON(page_has_private(page));
	__delete_from_page_cache(page);
	spin_unlock_irq(&mapping->tree_lock);
	mem_cgroup_uncharge_cache_page(page);

	if (mapping->a_ops->freepage)
		mapping->a_ops->freepage(page);

	page_cache_release(page);	/* pagecache ref */
	return 1;
failed:
	spin_unlock_irq(&mapping->tree_lock);
	return 0;
}

static int do_launder_page(struct address_space *mapping, struct page *page)
{
	if (!PageDirty(page))
		return 0;
	if (page->mapping != mapping || mapping->a_ops->launder_page == NULL)
		return 0;
	return mapping->a_ops->launder_page(page);
}

/**
 * invalidate_inode_pages2_range - remove range of pages from an address_space
 * @mapping: the address_space
 * @start: the page offset 'from' which to invalidate
 * @end: the page offset 'to' which to invalidate (inclusive)
 *
 * Any pages which are found to be mapped into pagetables are unmapped prior to
 * invalidation.
 *
 * Returns -EBUSY if any pages could not be invalidated.
 */
int invalidate_inode_pages2_range(struct address_space *mapping,
				  pgoff_t start, pgoff_t end)
{
	struct pagevec pvec;
	pgoff_t index;
	int i;
	int ret = 0;
	int ret2 = 0;
	int did_range_unmap = 0;

	cleancache_invalidate_inode(mapping);
	pagevec_init(&pvec, 0);
	index = start;
	while (index <= end && pagevec_lookup(&pvec, mapping, index,
			min(end - index, (pgoff_t)PAGEVEC_SIZE - 1) + 1)) {
		mem_cgroup_uncharge_start();
		for (i = 0; i < pagevec_count(&pvec); i++) {
			struct page *page = pvec.pages[i];

			/* We rely upon deletion not changing page->index */
			index = page->index;
			if (index > end)
				break;

			lock_page(page);
			WARN_ON(page->index != index);
			if (page->mapping != mapping) {
				unlock_page(page);
				continue;
			}
			wait_on_page_writeback(page);
			if (page_mapped(page)) {
				if (!did_range_unmap) {
					/*
					 * Zap the rest of the file in one hit.
					 */
					unmap_mapping_range(mapping,
					   (loff_t)index << PAGE_CACHE_SHIFT,
					   (loff_t)(1 + end - index)
							 << PAGE_CACHE_SHIFT,
					    0);
					did_range_unmap = 1;
				} else {
					/*
					 * Just zap this page
					 */
					unmap_mapping_range(mapping,
					   (loff_t)index << PAGE_CACHE_SHIFT,
					   PAGE_CACHE_SIZE, 0);
				}
			}
			BUG_ON(page_mapped(page));
			ret2 = do_launder_page(mapping, page);
			if (ret2 == 0) {
				if (!invalidate_complete_page2(mapping, page))
					ret2 = -EBUSY;
			}
			if (ret2 < 0)
				ret = ret2;
			unlock_page(page);
		}
		pagevec_release(&pvec);
		mem_cgroup_uncharge_end();
		cond_resched();
		index++;
	}
	cleancache_invalidate_inode(mapping);
	return ret;
}
EXPORT_SYMBOL_GPL(invalidate_inode_pages2_range);

/**
 * invalidate_inode_pages2 - remove all pages from an address_space
 * @mapping: the address_space
 *
 * Any pages which are found to be mapped into pagetables are unmapped prior to
 * invalidation.
 *
 * Returns -EBUSY if any pages could not be invalidated.
 */
int invalidate_inode_pages2(struct address_space *mapping)
{
	return invalidate_inode_pages2_range(mapping, 0, -1);
}
EXPORT_SYMBOL_GPL(invalidate_inode_pages2);

/**
 * truncate_pagecache - unmap and remove pagecache that has been truncated
 * @inode: inode
 * @newsize: new file size
 *
 * inode's new i_size must already be written before truncate_pagecache
 * is called.
 *
 * This function should typically be called before the filesystem
 * releases resources associated with the freed range (eg. deallocates
 * blocks). This way, pagecache will always stay logically coherent
 * with on-disk format, and the filesystem would not have to deal with
 * situations such as writepage being called for a page that has already
 * had its underlying blocks deallocated.
 */
void truncate_pagecache(struct inode *inode, loff_t newsize)
{
	struct address_space *mapping = inode->i_mapping;
	loff_t holebegin = round_up(newsize, PAGE_SIZE);

	/*
	 * unmap_mapping_range is called twice, first simply for
	 * efficiency so that truncate_inode_pages does fewer
	 * single-page unmaps.  However after this first call, and
	 * before truncate_inode_pages finishes, it is possible for
	 * private pages to be COWed, which remain after
	 * truncate_inode_pages finishes, hence the second
	 * unmap_mapping_range call must be made for correctness.
	 */
	unmap_mapping_range(mapping, holebegin, 0, 1);
	truncate_inode_pages(mapping, newsize);
	unmap_mapping_range(mapping, holebegin, 0, 1);
}
EXPORT_SYMBOL(truncate_pagecache);

/**
 * truncate_setsize - update inode and pagecache for a new file size
 * @inode: inode
 * @newsize: new file size
 *
 * truncate_setsize updates i_size and performs pagecache truncation (if
 * necessary) to @newsize. It will be typically be called from the filesystem's
 * setattr function when ATTR_SIZE is passed in.
 *
 * Must be called with inode_mutex held and before all filesystem specific
 * block truncation has been performed.
 */
void truncate_setsize(struct inode *inode, loff_t newsize)
{
	i_size_write(inode, newsize);
	truncate_pagecache(inode, newsize);
}
EXPORT_SYMBOL(truncate_setsize);

/**
 * truncate_pagecache_range - unmap and remove pagecache that is hole-punched
 * @inode: inode
 * @lstart: offset of beginning of hole
 * @lend: offset of last byte of hole
 *
 * This function should typically be called before the filesystem
 * releases resources associated with the freed range (eg. deallocates
 * blocks). This way, pagecache will always stay logically coherent
 * with on-disk format, and the filesystem would not have to deal with
 * situations such as writepage being called for a page that has already
 * had its underlying blocks deallocated.
 */
void truncate_pagecache_range(struct inode *inode, loff_t lstart, loff_t lend)
{
	struct address_space *mapping = inode->i_mapping;
	loff_t unmap_start = round_up(lstart, PAGE_SIZE);
	loff_t unmap_end = round_down(1 + lend, PAGE_SIZE) - 1;
	/*
	 * This rounding is currently just for example: unmap_mapping_range
	 * expands its hole outwards, whereas we want it to contract the hole
	 * inwards.  However, existing callers of truncate_pagecache_range are
	 * doing their own page rounding first.  Note that unmap_mapping_range
	 * allows holelen 0 for all, and we allow lend -1 for end of file.
	 */

	/*
	 * Unlike in truncate_pagecache, unmap_mapping_range is called only
	 * once (before truncating pagecache), and without "even_cows" flag:
	 * hole-punching should not remove private COWed pages from the hole.
	 */
	if ((u64)unmap_end > (u64)unmap_start)
		unmap_mapping_range(mapping, unmap_start,
				    1 + unmap_end - unmap_start, 0);
	truncate_inode_pages_range(mapping, lstart, lend);
}
EXPORT_SYMBOL(truncate_pagecache_range);
