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
pages_begin:
	page_t* p = first_page_;
	page_t** pp = &first_page_;

	while (p != nullptr)
	{
		// wait for in-progress page to be published or not
		if (p->id == pages_capacity_)
			goto pages_begin;

		if (p->size < p->capacity)
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
					atma::atomic_exchange(&p->id, pidx);
					page_slot_capacity_ *= 2;
				}
				else
				{
					// unpublish
					*pp = nullptr;
					delete p;
					goto pages_begin;
				}
			}
			else
			{
				delete p;
				goto pages_begin;
			}
		}
		else
		{
			goto pages_begin;
		}
	}
	else if (p->id == pages_capacity_)
	{
		goto pages_begin;
	}


	// get free slot in supposedly empty page
	uint32 idx = p->capacity;
	for (auto i = 0u, ie = p->capacity / 32; i != ie; ++i)
	{
		uint32 u32 = p->freeslots[i];
		if (u32 == 0xffffffff)
			continue;

freeslot_u32_retry:
		for (auto j = 0; j != 4; ++j)
		{
			byte b = (u32 >> ((3 - j) * 8)) & 0xff;

			uint32 k;
			if      ((b & 0b10000000) == 0) { k = 0; }
			else if ((b & 0b01000000) == 0) { k = 1; }
			else if ((b & 0b00100000) == 0) { k = 2; }
			else if ((b & 0b00010000) == 0) { k = 3; }
			else if ((b & 0b00001000) == 0) { k = 4; }
			else if ((b & 0b00000100) == 0) { k = 5; }
			else if ((b & 0b00000010) == 0) { k = 6; }
			else if ((b & 0b00000001) == 0) { k = 7; }
			else continue;

			auto nu32 = u32 | (0x80000000 >> (j * 8) >> k);

			if (atma::atomic_compare_exchange(&p->freeslots[i], u32, nu32, &u32))
			{
				atma::atomic_pre_increment(&p->size);
				idx = i * 32 + j * 8 + k;
				goto freeslot_end;
			}
			else
			{
				goto freeslot_u32_retry;
			}
		}
	}

	// page was filled whilst we were dawdling
	goto pages_begin;

freeslot_end:

	// construct now-owned slot
	p->memory.construct_default(idx, 1);

	//ATMA_ASSERT(p->memory[idx].ref_count == 0);
	return {p, idx};
}


auto lion::asset_library_t::release(std::tuple<uint32, uint32> const& s) -> void
{
	page_t* p = pages_[std::get<0>(s)];

	p->memory[std::get<1>(s)].ref_count--;

	uint32 idx = std::get<1>(s);
	uint32 i = idx / 32;
	uint32 j = idx % 32;

	uint32 u32 = p->freeslots[i];
	uint32 nu32;
	do {
		nu32 = u32 & ~(0x80000000 >> j);
	} while (!atma::atomic_compare_exchange(&p->freeslots[i], u32, nu32, &u32));

	atma::atomic_pre_decrement(&p->size);
}

auto lion::asset_library_t::dump_ascii() -> void
{
	for (int i = 0; i != pages_size_; ++i)
	{
		if (!pages_[i])
			continue;

		auto const* p = pages_[i];
		for (int j = 0; j != p->capacity; ++j)
		{
			if (p->freeslots[j / 32] & (0x80000000 >> (j % 32)))
				std::cout << pages_[i]->memory[j].ref_count;
			else
				std::cout << ".";
		}

		std::cout << " ";
	}

	std::cout << "E" << std::endl;
}

