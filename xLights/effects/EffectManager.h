#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H


#include <map>
#include <string>
#include <vector>

class RenderableEffect;

class EffectManager : public std::vector<RenderableEffect*>
{
    public:
        EffectManager();
        virtual ~EffectManager();
    
    
        RenderableEffect *GetEffect(int i) const {
            if (i >= size()) {
                return nullptr;
            }
            return (*this)[i];
        }
        RenderableEffect *GetEffect(const std::string &str) const;
        int GetLastEffectId() const { return size() - 1;};
    protected:
    private:
        void add(RenderableEffect *eff);
    
        mutable std::map<std::string, RenderableEffect *> effectsByName;
};

#endif // EFFECTMANAGER_H
