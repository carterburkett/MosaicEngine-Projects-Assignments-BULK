enum EmitterType { skinnyFire = 0, mobileCircle = 1, Rain = 2, rayGun = 3, rayGunPulse = 4 };

struct Particle {
    vec2 spawnPos;
    vec2 pos;
    vec2 velocity;
    vec2 endVel;
    vec2 dir;
    vec2 size;

    vec4 color;

    float lifetime;
    real32 timeAlive;

    float amp;
};

struct NoiseParams {
    float strength;
    float frequency;
    int amp;
    int speed;
};

//struct BurstParams {
//    int numParticles;
//    bool capRep;
//    int numRep;
//    float frequency;
//};

struct ParticleEmitter { //@TODO this should probably be broken up into more sub-structs or something. Ones for Vel, Size, dir, etc. But also
                            //compounding structs like that can be a headache sometimes.
    EmitterType type;
    DynamicArray<Particle*> particles;
    MemoryArena ParticleArena;

    int count;
    int capacity;
    float spawnRate;
    int amtSpawn;
    vec2 emitterPos;
    vec2 dir;
    vec2 dirMax;

    bool sizeRange;
    vec2 minSize;
    vec2 maxSize;
  
    bool velLifetime = false;
    bool velRange;
    vec2 velMin;
    vec2 velMax;
    
    bool spawnRange;
    vec2 sPosMin;
    vec2 sPosMax;

    bool sizeLifetime;
    bool sizeDistance;
    bool lifetimeRange;
    float pMinLifetime;
    float pMaxLifetime;

    bool lifetimeColor;
    bool distanceColor;
    vec4 startColor;
    vec4 endColor;
    
    real32 lastSpawnTime;
    real32 systemLifetime;
    real32 colorInDur;
    real32 colorOutDur;

    bool randDirection;
    bool dirRange = false;
    bool useBurst;
    //BurstParams burst;

    bool useNoise;
    NoiseParams noise;

    real32 moveTimer;
    real32 runTime;
};

ParticleEmitter* emitter = {};

real32 slerpTime = 0;
int emitterVal = 0;


//@INFO There's some weird and arbitary code in a couple of these functions. I know that best practice would be to completely avoid that so
// that the paramaters are entirely editable within the emitters. But for cases where I'm only using those paramters for a single type of emitter I just let it be
// in the interest of time
//There's also cases where things are named poorly. Sometimes the scope of what you think something will do changes as you code. I should have gone
//back and renamed stuff where it's appropriate but I'm in 27 credits and I'm honestly just too burnt out and too tired to seek all the cases out. I'm
// really sorry if it ends up being a nuisance. I hope you enjoy break :)
//        -Carter


//==========================================================================================
//                                     MATH
//==========================================================================================
vec2 Vec2RandiRange(vec2 min, vec2 max, int seed = NULL) { //@DEBUG this is messsed up. It's alternating b/w the same 2 outputs??
    if (seed != NULL) {
        SeedRand(seed);
    }

    vec2 out;
    out.x = RandiRange(min.x, max.x);
    out.y = RandiRange(min.y, max.y);
    return out;
}

vec2 Vec2RandfRange(vec2 min, vec2 max, int seed = NULL) {
    if (seed != NULL) {
        SeedRand(seed);
    }

    vec2 out;
    out.x = RandfRange(min.x, max.x);
    out.y = RandfRange(min.y, max.y);
    return out;
}

vec2 PointSlerp(vec2 start, vec2 end, float speed) {
    real32 t;
    

    emitter->moveTimer += DeltaTime;
    t = (emitter->moveTimer / speed);

    if (t >= 1 || t < 0) {
        emitter->moveTimer = 0;
        t = 0;
    }

    float ox = (start.x + end.x) / 2;
    float oy = (start.y + end.y) / 2;
    vec2 origin = V2(ox, oy);

    real32 rad = Distance(origin, start);

    float angle = Lerp(0, _2PI, t);

    float xCPos = (rad * cosf(angle - (_PI / 2))) + origin.x;
    float yCPos = (rad * sinf(angle - (_PI / 2))) + origin.y;

    vec2 circPos = V2(xCPos, yCPos);
    return circPos;
}


//==========================================================================================
//                                     INIT
//==========================================================================================
void InitMemory() {
    Mosaic->myData = malloc(sizeof(ParticleEmitter));
    memset(Mosaic->myData, 0, sizeof(ParticleEmitter));
    emitter = (ParticleEmitter*)Mosaic->myData;
}

void InitNoise(NoiseParams* n) {
    n->amp = 3.0f;
    n->strength = 4.0f;
}

void InitEmitter(EmitterType type) {
    //I am so sorry for this monstrosity. Outside of making a config file or something I have no idea how to prevent
    //some form of this when writing multiple instances of Particle Systems with unique behaviors.
    ParticleEmitter* e = new ParticleEmitter;
    if (type == skinnyFire) {
        e->capacity = 200;
        e->spawnRate = 0.25f;
        e->amtSpawn = 11;
        e->emitterPos = V2(32, 64);
        e->dirRange = false;
        e->dir = NORTH;

        e->lastSpawnTime = 0.0f;
        //e->colorInDur = 0.2f;
        //e->colorOutDur = 0.2f;
        e->systemLifetime = 30.0f;
        
        e->sizeRange = false; //all values with ranges will use minVals if paramRange is set to false.
        e->minSize = V2(1);

        e->velLifetime = false;
        e->velRange = true;
        e->velMin = V2(0.1f, 5);
        e->velMax = V2(0.5f, 25);

        e->lifetimeRange = true;
        e->pMinLifetime = 1.0f; //Don't put this below 1 if using noise. It's wonky if you do. @TODO debug whatever that is
        e->pMaxLifetime = 6.0f;

        e->sizeDistance = false;
        e->spawnRange = true;
        e->sPosMin = V2(-10,-1);
        e->sPosMax = V2(10,2);

        e->useBurst = false;
        e->useNoise = true;
        e->randDirection = false;
        e->sizeLifetime = false;
        e->distanceColor = false;

        e->lifetimeColor = true;
        e->startColor = RED;
        e->endColor = YELLOW;

        if (e->useNoise) { InitNoise(&e->noise); }

        emitter = e;
    }
    else if(type == mobileCircle) {
        e->capacity = 120;
        e->spawnRate = 0.05f;
        e->amtSpawn = 13;
        e->emitterPos = V2(32, 16);
        e->dirRange = false;
        e->dir = V2(0);

        e->lastSpawnTime = 0.0f;
        //e->colorInDur = 0.2f;
        //e->colorOutDur = 0.2f;
        e->systemLifetime = 30.0f;

        e->sizeRange = false; //all values with ranges will use minVals if paramRange is set to false.
        e->sizeLifetime = false;
        e->sizeDistance = false;
        e->minSize = V2(1);

        e->velRange = true;
        e->velLifetime = false;
        e->velMin = V2(-40);
        e->velMax = V2(40);

        e->lifetimeRange = true;
        e->pMinLifetime = 0.2f;
        e->pMaxLifetime = 0.5f;

        e->sizeDistance = false;
        e->randDirection = true;
        e->spawnRange = true;
        e->sPosMin = V2(-3);
        e->sPosMax = V2(3);

        e->useBurst = false;
        e->useNoise = false;

        e->distanceColor = false;
        e->lifetimeColor = true;
        e->startColor = V4(0.77f, 0.57f, 0.82f, 1.0f);
        e->endColor = V4(0.57f, 0.0f, 1.0f, 1.0f);

        if (e->useNoise) { InitNoise(&e->noise); }

        emitter = e;
    }
    else if (type == Rain) {
        e->capacity = 100;
        e->spawnRate = 0.2f;
        e->amtSpawn = 14;
        e->emitterPos = V2(32, 0);
        e->dirRange = false;
        e->dir = V2(0,1);

        e->lastSpawnTime = 0.0f;
        //e->colorInDur = 0.2f;
        //e->colorOutDur = 0.2f;
        e->systemLifetime = 60.0f;

        e->sizeRange = true; //all values with ranges will use minVals if paramRange is set to false.
        e->sizeLifetime = false;
        e->sizeDistance = true;
        e->minSize = V2(3); //should have probably named the start and endSize but I'm in too deep
        e->maxSize = V2(1);

        e->velRange = true;
        e->velLifetime = false;
        e->velMin = V2(0.4f,40);
        e->velMax = V2(0.6f, 90);

        e->lifetimeRange = true;
        e->pMinLifetime = 1.0f;
        e->pMaxLifetime = 2.0f;

        e->randDirection = false;
        e->spawnRange = true;
        e->sPosMin = V2(-32,-16);
        e->sPosMax = V2(32,1);

        e->useBurst = false;
        e->useNoise = false;

        e->lifetimeColor = false;
        e->distanceColor = true;
        e->startColor = V4(0.69f, 0.9f, 0.9f, 1.0f);
        e->endColor = V4(0.03f, 0.0f, 1.0f, 1.0f);

        if (e->useNoise) { InitNoise(&e->noise); }

        emitter = e;
    }
    else if(type == rayGun) {
        e->capacity = 300;
        e->spawnRate = 0.01f;
        e->amtSpawn = 25;
        e->emitterPos = V2(0, 32);
        e->dir = V2(-1, -1);
        e->dirMax = V2(-1, 1);

        e->lastSpawnTime = 0.0f;
        //e->colorInDur = 0.2f;
        //e->colorOutDur = 0.2f;
        e->systemLifetime = 45.0f;

        e->sizeRange = false; //all values with ranges will use minVals if paramRange is set to false.
        e->sizeLifetime = false;
        e->sizeDistance = false;
        e->minSize = V2(1); //should have probably named the start and endSize but I'm in too deep

        e->velLifetime = true;
        e->velRange = true;
        e->velMin = V2(50, 70);
        e->velMax = V2(80, 120);

        e->lifetimeRange = true;
        e->pMinLifetime = 0.1f;
        e->pMaxLifetime = 0.8f;

        e->randDirection = false;
        e->dirRange = true;

        e->spawnRange = true;
        e->sPosMin = V2(0, 0);
        e->sPosMax = V2(-2, 0);

        e->useBurst = false;
        e->useNoise = false;

        e->lifetimeColor = true;
        e->distanceColor = false;
        e->startColor = V4(0.00f, 1.00f, 0.33f, 1.0f);
        e->endColor = V4(0.03f, 0.08f, 0.02f, 1.0f);

        if (e->useNoise) { InitNoise(&e->noise); }

        emitter = e;

        
    }
    else if (type == rayGunPulse) {
        e->capacity = 300;
        e->spawnRate = 0.15f;
        e->amtSpawn = 25;
        e->emitterPos = V2(0, 32);
        e->dir = V2(-1, -1);
        e->dirMax = V2(-1, 1);

        e->lastSpawnTime = 0.0f;
        //e->colorInDur = 0.2f;
        //e->colorOutDur = 0.2f;
        e->systemLifetime = 45.0f;

        e->sizeRange = false; //all values with ranges will use minVals if paramRange is set to false.
        e->sizeLifetime = false;
        e->sizeDistance = false;
        e->minSize = V2(1); //should have probably named the start and endSize but I'm in too deep

        e->velLifetime = true;
        e->velRange = true;
        e->velMin = V2(50, 70);
        e->velMax = V2(80, 120);

        e->lifetimeRange = true;
        e->pMinLifetime = 0.1f;
        e->pMaxLifetime = 0.8f;

        e->randDirection = false;
        e->dirRange = true;

        e->spawnRange = true;
        e->sPosMin = V2(0, 0);
        e->sPosMax = V2(-2, 0);

        e->useBurst = false;
        e->useNoise = false;

        e->lifetimeColor = true;
        e->distanceColor = false;
        e->startColor = YELLOW;
        e->endColor = MAGENTA;

        if (e->useNoise) { InitNoise(&e->noise); }

        emitter = e;
        }
    else {
        Print("Please Input Your Emitter Type into Init Function");
    }
    emitter->count = 0;
    emitter->type = type;
    emitter->runTime = 0.0f;
    emitter->moveTimer = 0.0f;
    AllocateMemoryArena(&emitter->ParticleArena, sizeof(Particle)* emitter->capacity);
    emitter->particles = MakeDynamicArray<Particle*>(&emitter->ParticleArena, 2);
}

void InitParticle() {
    real32 t = Time - emitter->lastSpawnTime;
    real32 batchCount = 0;

    if (emitter->spawnRate < t) {
        for (int i = emitter->particles.count; i < emitter->capacity && batchCount < emitter->amtSpawn; i++) {
            Particle* p = new Particle;

            if (emitter->spawnRange) { p->pos = emitter->emitterPos + Vec2RandfRange(emitter->sPosMin, emitter->sPosMax); }
            else { p->pos = emitter->emitterPos; }
            p->spawnPos = p->pos;

            if (emitter->velRange) { p->velocity = Vec2RandfRange(emitter->velMin, emitter->velMax); }
            else { p->velocity = emitter->velMin; }
            p->endVel = p->velocity;


            if (emitter->lifetimeRange) { p->lifetime = RandfRange(emitter->pMinLifetime, emitter->pMaxLifetime); }
            else { p->lifetime = emitter->pMinLifetime; }

            if (emitter->sizeRange) { p->size = Vec2RandfRange(emitter->minSize, emitter->maxSize); }
            else { p->size = emitter->minSize; }

            if (emitter->dirRange) { p->dir = Vec2RandfRange(emitter->dir, emitter->dirMax); }
            else { p->dir = emitter->dir; }

            p->color = emitter->startColor;
            p->timeAlive = 0.0f;

            PushBack(&emitter->particles, p);
            emitter->count = emitter->particles.count;
            emitter->lastSpawnTime = Time;
            batchCount++;
        }
    }
}


//==========================================================================================
//                              Particle Manipulation
//==========================================================================================
void ParticleColorOverLifetime(Particle* p) {
    real32 t = p->timeAlive / p->lifetime;
    p->color = Lerp(emitter->startColor, emitter->endColor, t);
}

void ParticleColorOverDistance(Particle* p) {
    real32 d = InverseLerp(p->spawnPos.y, 128, p->pos.y);//@INFO more. arbitrary. numbers.
    p->color = Lerp(emitter->startColor, emitter->endColor, d); //@INFO This is a lazy way to do this. Maybe fix if there's time and you actually end up using it.
}

void ParticleSizeOverLifetime(Particle* p) {
    real32 t = p->timeAlive / p->lifetime;
    p->size = Lerp(emitter->minSize, emitter->maxSize, t);
}

void ParticleSizeOverDistance(Particle* p) { //@TODO fix so it takes in values from the emitter or particle not just arb nums
    real32 d = InverseLerp(p->spawnPos.y, 64, p->pos.y);
    p->size = Lerp(emitter->minSize, emitter->maxSize, d);
}

void ParticleVelocityOverLifetime(Particle* p) {
    real32 t = p->timeAlive / p->lifetime;
    p->velocity = Lerp(V2(0), p->endVel, t);
}

void AlterParticleAttributes(Particle* p) { //@TODO give this a better name
    if (emitter->lifetimeColor) { ParticleColorOverLifetime(p); }
    if (emitter->distanceColor) { ParticleColorOverDistance(p); }
    if (emitter->sizeDistance) { ParticleSizeOverDistance(p); }
    if (emitter->sizeLifetime) { ParticleSizeOverLifetime(p); }
    if (emitter->velLifetime) { ParticleVelocityOverLifetime(p); }
}

void ApplyNoise(Particle* p) {
    if (p->amp == NULL) { //@TODO make this better accomodate X and Y noise. Currently is Just X. May want to take direction into account so noise makes more sense depending on momentum
        p->amp = RandfRange(emitter->noise.amp - emitter->noise.strength, 
                                emitter->noise.amp + emitter->noise.strength);
    }
    p->pos.x = (cosf(p->timeAlive / p->velocity.x) * p->amp) + p->spawnPos.x;
    //@DEBUG I'm getting weird cases of one or two particles going buckwild when i call this function. It's literally just one or two of the 100 being spawned. I have no idea why.
}

void MoveParticle(Particle* p) {
    if (emitter->randDirection == true) {
        if (p->dir == V2(NULL)) {
            p->dir.x = RandfRange(-2, 2);
            p->dir.y = RandfRange(-2, 2);
            Print("%f", p->dir.x);
        }
    }

    //The code beneath this is arbitrary again.
    //You obviously would not want to format anything like this. What if i want noise on the y Axis or both axes?
    //What if I want to manipulate the positions in other ways?
    //This is bad code but it's also a case of doing so knowing it's to save time on an occasion where I need it.
    if (emitter->useNoise) { ApplyNoise(p); }
    else {
       p->pos.x += DeltaTime * p->dir.x * p->velocity.x;
    }

    p->pos.y += DeltaTime * p->dir.y * p->velocity.y;
    
}

void MoveEmitter() {
    //@TODO I don't run multiple emitters right now but if I ever do then this needs to recieve a pointer to an emitter...
    // //=============
    //@INFO. This doesn't manipulate the particles as much as it does the "emitter" which is basically just the origin/spawnLoc
    // //=================
    //@INFO This function is full of arbitrary numbers. It's also for two specific cases though. There's something to be said for avoiding instances of needlesly specific values
    //but at the same time this a specific case in an assignment that is moreso a proof of concept. It would make less sense for me to place varaibles into a struct that only
    //have one case of use
    if (emitter->type == mobileCircle) {
        emitter->emitterPos = PointSlerp(V2(16, 32), V2(48, 32), 4.0f); //I should probably have an actual variable for the speed since this a little
                                                                            //but atm this is the emitterType that uses it
    }
    if (emitter->type == rayGun || emitter->type == rayGunPulse) {

        emitter->moveTimer += DeltaTime;
        real32 t = emitter->moveTimer / 1.5f;

        if (t > 1.0f) {
            emitter->moveTimer = 0;
            t = 0.0f;
        }
        emitter->emitterPos = Lerp(V2(-16, 32), V2(96, 32), t);
    }
}

void CullParticles() {
    for (int i = 0; i < emitter->particles.count; i++) {
        Particle* p = emitter->particles[i];
        if (p->timeAlive >= p->lifetime) {
            RemoveAtIndex(&emitter->particles, i);
        }
    }
}


//==========================================================================================
//                                     Render
//==========================================================================================
void DrawParticles() {
    for (int i = 0; i < emitter->particles.count; i++) {
        Particle* p = emitter->particles[i];
        p->timeAlive += DeltaTime;
        MoveParticle(p);
        AlterParticleAttributes(p);

        if (p->size.x >= 2 || p->size.y >= 2) { SetBlockColor(p->pos, p->size.x, p->size.y, p->color); }
        else {
            SetTileColor(p->pos, p->color);
        }
    }
}

void CycleToNextSystem() {
    ClearMemoryArena(&emitter->ParticleArena);
    emitterVal++;
    if (emitterVal > 4) { emitterVal = 0; }
    InitEmitter((EmitterType)emitterVal);

    //I know we were technically supposed to use a maxParticles buffer so I'm breaking the rules here but I *think* this is an ok use for a DynamicArray
    //The size is inherently capped by the memoryArena. I could even make sure to cap it further by just giving a flat allocation to the Arena and then
    //preventing more particles from being drawn if the arena wouldn't be able to handle it. Again, I get that the point was to avoid allocation and
    //deallocation when it's not necessary but maybe there's an argument for not using more memory than you actively need?
}


//==========================================================================================
//                                     Mosaic
//==========================================================================================
void MyMosaicInit() {
    SetMosaicGridSize(64, 64);
    emitterVal = 0;

    InitMemory();
    InitEmitter((EmitterType)emitterVal);
    InitParticle();
}

void MyMosaicUpdate() {
    ClearTiles(BLACK);
    emitter->runTime += DeltaTime;

    if (emitter->systemLifetime >= emitter->runTime) {
        InitParticle();
        DrawParticles();
        CullParticles();
        MoveEmitter();
    }
    else {
        Print("ParticleSystem duration over. Moving to next System");
        CycleToNextSystem();
    }

    if (InputPressed(Keyboard, Input_Space)) {
        CycleToNextSystem();
    }
}
