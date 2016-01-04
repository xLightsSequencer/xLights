#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H


#include <map>
#include <string>

class RenderableEffect;

class EffectManager
{
    public:
        EffectManager();
        virtual ~EffectManager();
    
    
        RenderableEffect *GetEffect(int i) const;
        RenderableEffect *GetEffect(const std::string &str) const;
        int GetLastEffectId() const;
    protected:
    private:
    
    
    mutable std::map<int, RenderableEffect *> effectsById;
    mutable std::map<std::string, RenderableEffect *> effectsByName;
    int last;
};

#endif // EFFECTMANAGER_H
