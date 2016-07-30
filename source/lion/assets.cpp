#include <lion/assets.hpp>

lion::asset_library_t::asset_library_t()
	//: types_{256}
{
	
}

auto lion::asset_library_t::gen_random() -> std::tuple<uint32, uint32>
{
	auto s = get_slot();

	auto page = std::get<0>(s);
	auto idx = std::get<1>(s);

	ATMA_ASSERT(page->memory[idx].ref_count == 0);
	page->memory[idx].ref_count++;

	return {std::get<0>(s)->id, std::get<1>(s)};
}

auto lion::asset_library_t::get_slot() -> std::tuple<page_t*, uint32>
{
begin:
	page_t* p = first_page_;
	page_t** pp = &first_page_;

	while (p != nullptr)
	{
		// wait for in-progress page to be published or not
		if (p->id == 256)
			goto begin;

		if (p->idx < page_slot_capacity_ || p->freelist.node != nullptr)
			break;

		pp = &p->next;
		p = p->next;
	}

	if (p == nullptr)
	{
		// allocate to pp
		if (pages_size_ < pages_capacity_)
		{
			p = new page_t{pages_capacity_, page_slot_capacity_};

			// insert page at end of chain, but don't publish it until we've
			// established there's enough space in the table
			if (atma::atomic_compare_exchange<page_t*>(pp, nullptr, p))
			{
				auto pidx = atma::atomic_post_increment<uint32>(&pages_size_);
				if (pidx < pages_capacity_)
				{
					// publish
					pages_[pidx] = p;
					atma::atomic_exchange(&p->id, pidx); // p->id = pidx;
				}
				else
				{
					// unpublish
					*pp = nullptr;
					delete p;
					goto begin;
				}
			}
			else
			{
				delete p;
				goto begin;
			}
		}
		else
			goto begin;
	}
	else if (p->id == pages_capacity_)
	{
		goto begin;
	}


	uint32 idx = page_slot_capacity_;
	if (p->idx < page_slot_capacity_)
	{
		idx = atma::atomic_post_increment(&p->idx);
	}

	if (idx >= page_slot_capacity_)
	{
		freelist_head_t ch;
		atma::atomic_load_128(&ch, &p->freelist);

		while (ch.node)
		{
			freelist_head_t nh{
				ch.node->next.load(std::memory_order_relaxed),
				ch.aba + 1};

			if (atma::atomic_compare_exchange(&p->freelist, ch, nh, &ch))
			{
				idx = ch.node->idx;
				delete ch.node;
				break;
			}
		}
	}

	if (idx >= page_slot_capacity_)
		goto begin;

	p->memory.construct_default(idx, 1);
	ATMA_ASSERT(p->memory[idx].ref_count == 0);
	return {p, idx};
}


auto lion::asset_library_t::release(std::tuple<uint32, uint32> const& s) -> void
{
	page_t* p = pages_[std::get<0>(s)];

	p->memory[std::get<1>(s)].ref_count--;

	freelist_head_t ch;
	atma::atomic_load_128(&ch, &p->freelist);

	freelist_head_t nh;
	do {
		delete nh.node;
		nh.node = new freelist_node_t{std::get<1>(s), ch.node};
		nh.aba = ch.aba + 1;
	} while (!atma::atomic_compare_exchange(&p->freelist, ch, nh, &ch));
}

auto lion::asset_library_t::dump_ascii() -> void
{
	for (int i = 0; i != pages_size_; ++i)
	{
		if (!pages_[i])
			continue;

		for (int j = 0; j != page_slot_capacity_ && j != pages_[i]->idx; ++j)
		{
			std::cout << pages_[i]->memory[j].ref_count;
		}

		std::cout << " ";
	}

	std::cout << "E" << std::endl;
}

