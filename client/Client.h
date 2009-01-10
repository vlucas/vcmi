#ifndef __CLIENT_H__
#define __CLIENT_H__


#include "../global.h"
#include <boost/thread.hpp>
#include "../lib/IGameCallback.h"
struct StartInfo;
class CGameState;
class CGameInterface;
class CConnection;
class CCallback;
class CClient;
void processCommand(const std::string &message, CClient *&client);
namespace boost
{
	class mutex;
	class condition_variable;
}

template <typename T>
struct CSharedCond
{
	boost::mutex *mx;
	boost::condition_variable *cv;
	T *res;
	CSharedCond(T*R)
	{
		res = R;
		mx = new boost::mutex;
		cv = new boost::condition_variable;
	}
	~CSharedCond()
	{
		delete res;
		delete mx;
		delete cv;
	}
};

class CClient : public IGameCallback
{
	CCallback *cb;
	std::map<ui8,CGameInterface *> playerint;
	CConnection *serv;

	void waitForMoveAndSend(int color);
public:
	CClient(void);
	CClient(CConnection *con, StartInfo *si);
	~CClient(void);

	void close();
	void newGame(CConnection *con, StartInfo *si); //con - connection to server
	void save(const std::string & fname);
	void load(const std::string & fname);
	void process(int what);
	void run();
	//////////////////////////////////////////////////////////////////////////
	//from IGameCallback
	int getCurrentPlayer();
	int getSelectedHero();

	//not working yet, will be implement somewhen later with support for local-sim-based gameplay
	void changeSpells(int hid, bool give, const std::set<ui32> &spells){};
	void removeObject(int objid){};
	void setBlockVis(int objid, bool bv){};
	void setOwner(int objid, ui8 owner){};
	void setHoverName(int objid, MetaString * name){};
	void setObjProperty(int objid, int prop, int val){};
	void changePrimSkill(int ID, int which, int val, bool abs=false){};
	void changeSecSkill(int ID, int which, int val, bool abs=false){}; 
	void showInfoDialog(InfoWindow *iw){};
	void showYesNoDialog(YesNoDialog *iw, const CFunctionList<void(ui32)> &callback){};
	void showSelectionDialog(SelectionDialog *iw, const CFunctionList<void(ui32)> &callback){}; //returns question id
	void giveResource(int player, int which, int val){};
	void showCompInfo(ShowInInfobox * comp){};
	void heroVisitCastle(int obj, int heroID){};
	void stopHeroVisitCastle(int obj, int heroID){};
	void giveHeroArtifact(int artid, int hid, int position){}; //pos==-1 - first free slot in backpack=0; pos==-2 - default if available or backpack
	void startBattleI(const CCreatureSet * army1, const CCreatureSet * army2, int3 tile, const CGHeroInstance *hero1, const CGHeroInstance *hero2, boost::function<void(BattleResult*)> cb){}; //use hero=NULL for no hero
	void startBattleI(int heroID, CCreatureSet army, int3 tile, boost::function<void(BattleResult*)> cb){}; //for hero<=>neutral army
	void setAmount(int objid, ui32 val){};
	void moveHero(int hid, int3 pos, bool instant){};
	//////////////////////////////////////////////////////////////////////////
	friend class CCallback; //handling players actions
	friend void processCommand(const std::string &message, CClient *&client); //handling console
	
	
	static void runServer(const char * portc);
	static void waitForServer();
};

#endif // __CLIENT_H__
