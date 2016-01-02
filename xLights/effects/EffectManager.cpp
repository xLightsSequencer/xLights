#include "EffectManager.h"


#include "../BitmapCache.h"

#include "OffEffect.h"
#include "OnEffect.h"
#include "BarsEffect.h"


EffectManager::EffectManager()
{
    effects[BitmapCache::eff_OFF] = new OffEffect(BitmapCache::eff_OFF);
    effects[BitmapCache::eff_ON] = new OnEffect(BitmapCache::eff_ON);
    effects[BitmapCache::eff_BARS] = new BarsEffect(BitmapCache::eff_BARS);
}

EffectManager::~EffectManager()
{
    //dtor
}



RenderableEffect *EffectManager::GetEffect(int i) {
    return effects[i];
}