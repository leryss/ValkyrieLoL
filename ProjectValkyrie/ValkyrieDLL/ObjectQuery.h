#pragma once
#include "GameState.h"


using namespace boost::python;

class QCondition {

public:
	virtual bool Check(const GameObject* obj) = 0;
};

class QConditionTeam: public QCondition {

public:
	virtual bool Check(const GameObject* obj);

	int team;
	bool mustEqual;
};

class QConditionOnScreen : public QCondition {

public:
	virtual bool Check(const GameObject* obj);

	const GameRenderer* renderer;
};

class QConditionNearbyPoint : public QCondition {

public:
	virtual bool Check(const GameObject* obj);

	Vector3 point;
	float distance;
};

class QConditionTargetable : public QCondition {
public:
	virtual bool Check(const GameObject* obj);

	bool targetable;
};

class QConditionDeath : public QCondition {
public:
	virtual bool Check(const GameObject* obj);

	bool death;
};

class QConditionClone : public QCondition {
public:
	virtual bool Check(const GameObject* obj);

	bool clone;
};

class QConditionVisibility : public QCondition {
public:
	virtual bool Check(const GameObject* obj);

	bool visible;
};

class QConditionCasting : public QCondition {
public:
	virtual bool Check(const GameObject* obj);

	bool casting;
};

class QConditionHasTag : public QCondition {
public:
	virtual bool Check(const GameObject* obj);
	
	UnitTag tag;
};

class QueryException : public std::exception {
public:
	using std::exception::exception;
};

enum QueryKey {
	QKEY_MINION   = 0,
	QKEY_TURRET   = 1,
	QKEY_JUNGLE   = 2,
	QKEY_MISSILE  = 3,
	QKEY_CHAMP    = 4,
	QKEY_OTHERS   = 5,
	QKEY_NUM_KEYS = 6
};

class ObjectQuery {

public:
	ObjectQuery();


	void                         Update(const GameState* state);
				                 
	void                         NewQuery(QueryKey key);
	list                         GetResultsPy();	
	int                          Count();
						         
	ObjectQuery*                 AllyTo(const GameObject& obj);
	ObjectQuery*                 EnemyTo(const GameObject& obj);
	ObjectQuery*                 NearObj(const GameObject& obj, float distance);
	ObjectQuery*                 NearPoint(const Vector3& pt, float distance);
	ObjectQuery*                 HasTag(UnitTag tag);
	ObjectQuery*                 Targetable();
	ObjectQuery*                 Untargetable();
	ObjectQuery*                 Visible();
	ObjectQuery*                 Invisible();
	ObjectQuery*                 Alive();
	ObjectQuery*                 Dead();
	ObjectQuery*                 IsClone();
	ObjectQuery*                 IsNotClone();
	ObjectQuery*                 IsCasting();
						         
	ObjectQuery*                 OnScreen();

protected:
	template <class T>
	void MakePyObjects(QueryKey key, const std::vector<std::shared_ptr<T>>& objs);

	template <class T>
	list MakePyList(const std::vector<object>& pyObjs, const std::vector<std::shared_ptr<T>>& objs);

	template <class T>
	int  CountQuery(const std::vector<object>& pyObjs, const std::vector<std::shared_ptr<T>>& objs);

private:
	QueryKey                 qkey;
	std::vector<QCondition*> conditions;
	std::vector<object>      pyObjects[QueryKey::QKEY_NUM_KEYS];

	QConditionTeam           teamCondition;
	QConditionOnScreen       onScreenCondition;
	QConditionNearbyPoint    nearbyPointCondition;
	QConditionTargetable     targetableCondition;
	QConditionDeath          deathCondition;
	QConditionClone          cloneCondition;
	QConditionVisibility     visibilityCondition;
	QConditionCasting        castingCondition;
	QConditionHasTag         conditionHasTag;

	const GameState*         state;
};


template<class T>
inline void ObjectQuery::MakePyObjects(QueryKey key, const std::vector<std::shared_ptr<T>>& objs)
{
	auto& pyList = pyObjects[key];
	pyList.clear();
	for (auto& o : objs) {
		pyList.push_back(object(ptr(o.get())));
	}
}

template<class T>
inline list ObjectQuery::MakePyList(const std::vector<object>& pyObjs, const std::vector<std::shared_ptr<T>>& objs)
{
	list l = list();
	for (size_t i = 0; i < objs.size(); ++i) {

		bool belongs = true;
		for (auto& c : conditions) {
			if (!c->Check(objs[i].get())) {
				belongs = false;
				break;
			}
		}

		if (belongs)
			l.append(pyObjs[i]);
	}

	return l;
}

template<class T>
inline int ObjectQuery::CountQuery(const std::vector<object>& pyObjs, const std::vector<std::shared_ptr<T>>& objs)
{
	int count = 0;
	for (size_t i = 0; i < objs.size(); ++i) {

		bool belongs = true;
		for (auto& c : conditions) {
			if (!c->Check(objs[i].get())) {
				belongs = false;
				break;
			}
		}

		if (belongs)
			count += 1;
	}

	return count;
}
