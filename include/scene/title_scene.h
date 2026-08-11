#ifndef TITLE_SCENE_H
#define TITLE_SCENE_H

#include "bn_sprite_ptr.h"
#include "bn_vector.h"

class TitleScene
{
public:
    TitleScene();

    [[nodiscard]] bool update() const;
    void hide();
    void show();

private:
    bn::vector<bn::sprite_ptr, 32> _sprites;
};

#endif
