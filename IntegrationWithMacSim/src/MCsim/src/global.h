#ifdef CMD_TRACE_ENABLED
#define TRACE_CMD(str) std::cerr<< str <<std::endl;
#else
#define TRACE_CMD(str) 
#endif

#ifdef REQ_TRACE_ENABLED
#define TRACE_REQ(str) std::cerr<< str <<std::endl;
#else
#define TRACE_REQ(str) 
#endif

#ifdef DEBUG_ENABLED
#define DEBUG(str) std::cerr<< str <<std::endl;
#else
#define DEBUG(str) 
#endif

#define PRINT(str) std::cerr<< str <<std::endl;
#define ERROR(str) std::cerr<<"[ERROR ("<<__FILE__<<":"<<__LINE__<<")]: "<<str<<std::endl;

namespace MCsim
{
	// Memory Organization Table
	enum memOrg
	{
		Rank, BankGroup, Bank, SubArray, Row, Column
	};
}