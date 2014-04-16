#include "HelloWorldScene.h"

USING_NS_CC;

HelloWorld::HelloWorld() :_ptrPhysicsWorld(NULL)
, _ptrDebugDraw(NULL)
, m_groundBody(NULL)
, armFixture(NULL)
, armBody(NULL)
, armJoint(NULL)
, m_mouseJoint(NULL)
, m_currentBullet(0)
, m_releaseingArm(false)
{

}

HelloWorld::~HelloWorld()
{

}

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(HelloWorld::menuCloseCallback));
    
	pCloseItem->setPosition(ccp(origin.x + visibleSize.width - pCloseItem->getContentSize().width/2 ,
                                origin.y + pCloseItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu, 1);

	CCSprite* sprite = NULL;

	sprite = CCSprite::create("images/bg.png");
	sprite->setAnchorPoint(CCPointZero);
	addChild(sprite, -1);


	sprite = CCSprite::create("images/catapult_base_2.png"); //投射器底部后面那块 
	sprite->setAnchorPoint(CCPointZero);
	sprite->setPosition(CCPointMake(181.0, FLOOR_HEIGHT));
	this->addChild(sprite, 0);

	sprite = CCSprite::create("images/squirrel_1.png");        //左边松鼠 
	sprite->setAnchorPoint(CCPointZero);
	sprite->setPosition(CCPointMake(11.0, FLOOR_HEIGHT));
	this->addChild(sprite, 0);

	sprite = CCSprite::create("images/catapult_base_1.png");   //投射器底部前面那块 
	sprite->setAnchorPoint(CCPointZero);
	sprite->setPosition(CCPointMake(181.0, FLOOR_HEIGHT));
	this->addChild(sprite, 9);

	sprite = CCSprite::create("images/squirrel_2.png");    //右边松鼠 
	sprite->setAnchorPoint(CCPointZero);
	sprite->setPosition(CCPointMake(240.0, FLOOR_HEIGHT));
	this->addChild(sprite, 9);

	sprite = CCSprite::create("images/fg.png");    //带冰的地面 
	sprite->setAnchorPoint(CCPointZero);
	//this->addChild(sprite, 10);

	initPhysics();

	initArm();

	CCCallFunc *callSelectorAction = CCCallFunc::create(this, callfunc_selector(HelloWorld::resetGame));
	this->runAction(CCSequence::create(CCDelayTime::create(0.2f),
		callSelectorAction,
		NULL));

	scheduleUpdate();
    
    return true;
}


void HelloWorld::menuCloseCallback(CCObject* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	CCMessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
#else
    CCDirector::sharedDirector()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
#endif
}

void HelloWorld::initPhysics()
{
	b2Vec2 gravity;
	gravity.Set(0.0f, -10.0f);
	_ptrPhysicsWorld = new b2World(gravity);
	_ptrPhysicsWorld->SetAllowSleeping(true);
	_ptrPhysicsWorld->SetContinuousPhysics(true);

	_ptrDebugDraw = new GLESDebugDraw(PTM_RATIO);
	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;
	flags += b2Draw::e_jointBit;
	flags += b2Draw::e_aabbBit;
	flags += b2Draw::e_pairBit;
	flags += b2Draw::e_centerOfMassBit;
	_ptrDebugDraw->SetFlags(flags);

	_ptrPhysicsWorld->SetDebugDraw(_ptrDebugDraw);

	b2BodyDef grounddef;
	grounddef.position.Set(0, 0);

	m_groundBody = _ptrPhysicsWorld->CreateBody(&grounddef);

	CCSize screenSize = CCDirector::sharedDirector()->getVisibleSize();

	b2EdgeShape groundBox;
	// bottom 
	groundBox.Set(b2Vec2(0, FLOOR_HEIGHT / PTM_RATIO), b2Vec2(screenSize.width*2.0f / PTM_RATIO, FLOOR_HEIGHT / PTM_RATIO));
	m_groundBody->CreateFixture(&groundBox, 0);

	// top 
	groundBox.Set(b2Vec2(0, screenSize.height / PTM_RATIO), b2Vec2(screenSize.width*2.0f / PTM_RATIO, screenSize.height / PTM_RATIO));
	m_groundBody->CreateFixture(&groundBox, 0);

	// left 
	groundBox.Set(b2Vec2(0, screenSize.height / PTM_RATIO), b2Vec2(0, 0));
	m_groundBody->CreateFixture(&groundBox, 0);

	// right 
	groundBox.Set(b2Vec2(screenSize.width*1.5f / PTM_RATIO, screenSize.height / PTM_RATIO), b2Vec2(screenSize.width*1.5f / PTM_RATIO, 0));
	m_groundBody->CreateFixture(&groundBox, 0);
}

void HelloWorld::update(float delta)
{
	_ptrPhysicsWorld->Step(delta, 8, 3);

	for (b2Body* b = _ptrPhysicsWorld->GetBodyList(); b; b = b->GetNext())
	{
		if (b->GetUserData() != NULL)
		{
			CCSprite* myActor = (CCSprite*)b->GetUserData();
			myActor->setPosition(CCPointMake(b->GetPosition().x*PTM_RATIO,
				b->GetPosition().y*PTM_RATIO));
			myActor->setRotation(-1 * CC_RADIANS_TO_DEGREES(b->GetAngle()));

		}
	}

	if (m_releaseingArm && m_bulletJoint != NULL)
	{
		if (armJoint->GetJointAngle() < CC_DEGREES_TO_RADIANS(10))
		{
			m_releaseingArm = false;
			_ptrPhysicsWorld->DestroyJoint(m_bulletJoint);
			m_bulletJoint = NULL;
		}
	}

	if (m_bulletBody && m_bulletJoint == NULL)
	{
		b2Vec2 position = m_bulletBody->GetPosition();
		CCPoint myPosition = this->getPosition();
		CCSize screenSize = CCDirector::sharedDirector()->getWinSize();

		// Move the camera. 
		if (position.x > screenSize.width / 2.0f / PTM_RATIO)
		{
			myPosition.x = -MIN(screenSize.width * 2.0f - screenSize.width, position.x * PTM_RATIO - screenSize.width / 2.0f);
			this->setPosition(myPosition);
		}
	}
}

void HelloWorld::draw()
{
	ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position);

	kmGLPushMatrix();

	_ptrPhysicsWorld->DrawDebugData();

	kmGLPopMatrix();
}

void HelloWorld::initArm()
{
	CCSprite* arm = CCSprite::create("images/catapult_arm.png");
	this->addChild(arm, 1);

	b2BodyDef armdef;
	armdef.type = b2_dynamicBody;
	//线性阻尼
	armdef.linearDamping = 1;
	//转动阻尼
	armdef.angularDamping = 1;
	armdef.position.Set(230.0f / PTM_RATIO, (FLOOR_HEIGHT + 91.0f) / PTM_RATIO);
	armdef.userData = arm;
	armBody = _ptrPhysicsWorld->CreateBody(&armdef);

	b2PolygonShape armBox;
	b2FixtureDef fixturedef;
	fixturedef.shape = &armBox;
	fixturedef.density = 0.3f;
	armBox.SetAsBox(11.0f / PTM_RATIO, 91.0f / PTM_RATIO);
	armFixture = armBody->CreateFixture(&fixturedef);

	//init arm revoluteJoint
	b2RevoluteJointDef armJointDef;
	armJointDef.Initialize(m_groundBody, armBody, b2Vec2(233.0f / PTM_RATIO, FLOOR_HEIGHT / PTM_RATIO));
	armJointDef.enableLimit = true;
	armJointDef.enableMotor = true;
	armJointDef.motorSpeed = -10;
	armJointDef.lowerAngle = CC_DEGREES_TO_RADIANS(9);
	armJointDef.upperAngle = CC_DEGREES_TO_RADIANS(60);
	armJointDef.maxMotorTorque = 4800;

	armJoint = (b2RevoluteJoint*)_ptrPhysicsWorld->CreateJoint(&armJointDef);
}

void HelloWorld::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)
{
	if (m_mouseJoint != NULL)
		return;

	CCTouch* touch = (CCTouch*)pTouches->anyObject();
	CCPoint location = touch->getLocationInView();
	location = CCDirector::sharedDirector()->convertToGL(location);
	b2Vec2 locationWorld = b2Vec2(location.x / PTM_RATIO, location.y / PTM_RATIO);

	if (locationWorld.x < armBody->GetWorldCenter().x + 150.0 / PTM_RATIO)
	{
		b2MouseJointDef md;
		md.bodyA = m_groundBody;
		md.bodyB = armBody;
		md.target = locationWorld;
		md.maxForce = 2000;

		m_mouseJoint = (b2MouseJoint*)_ptrPhysicsWorld->CreateJoint(&md);
	}
}

void HelloWorld::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)
{
	if (m_mouseJoint == NULL)   { return; }

	CCTouch *touch = (CCTouch *)pTouches->anyObject();
	CCPoint location = touch->getLocationInView();
	location = CCDirector::sharedDirector()->convertToGL(location);
	b2Vec2 locationWorld = b2Vec2(location.x / PTM_RATIO, location.y / PTM_RATIO);

	m_mouseJoint->SetTarget(locationWorld);
}

void HelloWorld::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent)
{
	if (m_mouseJoint != NULL)
	{
		if (m_mouseJoint != NULL)
		{
			if (armJoint->GetJointAngle() >= CC_DEGREES_TO_RADIANS(5))
			{
				m_releaseingArm = true;
			}
		}
		_ptrPhysicsWorld->DestroyJoint(m_mouseJoint);
		m_mouseJoint = NULL;
		return;
	}
}

void HelloWorld::registerWithTouchDispatcher()
{
	CCDirector::sharedDirector()->getTouchDispatcher()->addStandardDelegate(this, 0);
}

void HelloWorld::onEnter()
{
	this->setTouchEnabled(true);
	CCLayer::onEnter();
}

void HelloWorld::onExit()
{
	CCLayer::onExit();
	this->setTouchEnabled(false);
}

void HelloWorld::createBullets(int count)
{
	m_currentBullet = 0;
	float pos = 62.0f;

	if (count > 0)
	{
		// delta is the spacing between corns 
		// 62 is the position o the screen where we want the corns to start appearing 
		// 165 is the position on the screen where we want the corns to stop appearing 
		// 30 is the size of the corn 

		float delta = (count > 1) ? ((165.0f - 62.0f - 30.0f) / (count - 1)) : 0.0f;

		for (int i = 0; i < count; i++, pos += delta)
		{
			// Create the bullet 
			CCSprite *sprite = CCSprite::create("images/acorn.png");
			this->addChild(sprite, 1);

			b2BodyDef bulletBodyDef;
			bulletBodyDef.type = b2_dynamicBody;
			bulletBodyDef.bullet = true;
			bulletBodyDef.position.Set(pos / PTM_RATIO, (FLOOR_HEIGHT + 15.0f) / PTM_RATIO);
			bulletBodyDef.userData = sprite;
			b2Body *bullet = _ptrPhysicsWorld->CreateBody(&bulletBodyDef);
			bullet->SetActive(false);

			b2CircleShape circle;
			circle.m_radius = 15.0 / PTM_RATIO;

			b2FixtureDef ballShapeDef;
			ballShapeDef.shape = &circle;
			ballShapeDef.density = 0.8f;
			ballShapeDef.restitution = 0.2f;
			ballShapeDef.friction = 0.99f;
			bullet->CreateFixture(&ballShapeDef);

			m_bullets.push_back(bullet);
		}
	}
}

bool HelloWorld::attachBullet()
{
	if (m_currentBullet < m_bullets.size())
	{
		m_bulletBody = (b2Body*)m_bullets.at(m_currentBullet++);
		m_bulletBody->SetTransform(b2Vec2(230.0f / PTM_RATIO, (155.0f + FLOOR_HEIGHT) / PTM_RATIO), 0.0f);
		m_bulletBody->SetActive(true);

		b2WeldJointDef weldJointDef;
		weldJointDef.Initialize(m_bulletBody, armBody, b2Vec2(230.0f / PTM_RATIO, (155.0f + FLOOR_HEIGHT) / PTM_RATIO));
		weldJointDef.collideConnected = false;

		m_bulletJoint = (b2WeldJoint*)_ptrPhysicsWorld->CreateJoint(&weldJointDef);
		return true;
	}
	return false;
}

void HelloWorld::resetGame()
{
	this->createBullets(4);
	this->attachBullet();
}















