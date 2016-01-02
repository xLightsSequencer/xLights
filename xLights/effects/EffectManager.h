#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H


#include <map>

class RenderableEffect;

class EffectManager
{
    public:
        EffectManager();
        virtual ~EffectManager();
    
    
        RenderableEffect *GetEffect(int i);
    
    protected:
    private:
    
    
    std::map<int, RenderableEffect *> effects;
};

#endif // EFFECTMANAGER_H
