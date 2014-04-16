#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
USING_NS_CC;
#include "Utils/GLES-Render.h"

#define FLOOR_HEIGHT 62.0f
#define PTM_RATIO 32.0f

class HelloWorld : public cocos2d::CCLayer
{
public:
	HelloWorld();
	~HelloWorld();
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::CCScene* scene();
    
    // a selector callback
    void menuCloseCallback(CCObject* pSender);
    
    // implement the "static node()" method manually
    CREATE_FUNC(HelloWorld);

	void initPhysics();
	void initArm();

	virtual void update(float delta);
	virtual void draw();

	virtual void registerWithTouchDispatcher();

	virtual void onEnter();
	virtual void onExit();

	virtual void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent);

	void createBullets(int count);
	bool attachBullet();
	void resetGame();

private:
	b2World* _ptrPhysicsWorld;
	GLESDebugDraw* _ptrDebugDraw;
	b2Body* m_groundBody;
	b2Fixture *armFixture;
	b2Body *armBody;
	b2RevoluteJoint* armJoint;
	b2MouseJoint* m_mouseJoint;

	std::vector<b2Body*> m_bullets;
	int m_currentBullet;

	b2Body* m_bulletBody;
	b2WeldJoint* m_bulletJoint;
	bool m_releaseingArm;
};

#endif // __HELLOWORLD_SCENE_H__
