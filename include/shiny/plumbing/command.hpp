#ifndef SHINY_PLUMBING_COMMAND_HPP
#define SHINY_PLUMBING_COMMAND_HPP
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	struct command_t
	{
		command_t() : finished(), destroyable() {}
		virtual ~command_t() {}
		
		virtual bool operator ()() = 0;

		bool finished;
		bool destroyable;
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
