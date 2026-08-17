#ifndef CHARACTER_DEFINITION_H
#define CHARACTER_DEFINITION_H

enum class CharacterId
{
    SAMURAI,
    GOBLIN,
    CROSSBOW_GOBLIN
};

struct CharacterDefinition
{
    CharacterId id;
    const char* display_name;
    int display_name_length;
    int max_hp;
};

#endif
