#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <random>
#include <set>
#include <sstream>
#include <cmath>

using namespace geode::prelude;

// ---------------------------------------------------------------
// Objects the mod NEVER creates:
// text (914), triggers, portals, spikes and saws.
// ---------------------------------------------------------------
static const std::set<int> BLOCKED = {
    914,                                             // text object
    29, 30, 104, 105, 221, 717, 718, 743, 744, 915,  // color triggers
    899, 901, 1006, 1007, 1049, 1268, 1346, 1347,
    1520, 1595, 1611, 1612, 1616, 1811, 1812, 1815,
    1817, 1818, 1819, 1912, 1913, 1914, 1915, 2062,
    10, 11, 12, 13, 45, 46, 47, 99, 101, 111, 286, 287,
    660, 745, 747, 749, 1331,                        // portals
    8, 9, 39, 61, 88, 89, 98, 103, 135, 145, 177,    // spikes / saws
    178, 179, 186, 187, 188, 189, 190, 191, 192, 193,
    392, 394, 395, 396, 678, 679, 680, 718, 1705,
    1706, 1707, 1708, 1709, 1710
};

enum class Rot { None, Random, Snap90, Diag45 };
enum class Pat { Edges, Corners, Scatter, Rain, Wave, Grid };

struct Preset {
    const char* name;
    std::vector<int> ids;
    int density;      // chance per position, %
    float minS, maxS; // size
    Rot rot;
    Pat pat;
};

static const std::vector<Preset>& presets() {
    static const std::vector<Preset> p = {
        // ---- MODERN ----
        {"Modern Clean",       {1},             30, .4f, .6f, Rot::Snap90, Pat::Edges},
        {"Modern Grid",        {1, 2},          45, .3f, .3f, Rot::None,   Pat::Grid},
        {"Modern Slabs",       {40},            40, .5f, 1.f, Rot::Snap90, Pat::Edges},
        {"Modern Diamonds",    {1},             35, .3f, .6f, Rot::Diag45, Pat::Corners},
        // ---- TECHNO ----
        {"Techno Pixels",      {1, 2, 3},       55, .2f, .35f, Rot::None,  Pat::Grid},
        {"Techno Circuit",     {40, 1},         50, .3f, .8f, Rot::Snap90, Pat::Edges},
        {"Techno Matrix",      {1},             60, .2f, .3f, Rot::None,   Pat::Rain},
        {"Techno Neon Bars",   {40},            45, .6f, 1.2f, Rot::None,  Pat::Rain},
        {"Techno Glitch",      {1, 40},         40, .2f, 1.f, Rot::Random, Pat::Scatter},
        {"Techno Grid Pro",    {3, 4},          50, .25f, .4f, Rot::None,  Pat::Grid},
        // ---- BEACH ----
        {"Beach Waves",        {40},            60, .5f, .9f, Rot::None,   Pat::Wave},
        {"Beach Sand",         {1, 2},          70, .15f, .3f, Rot::Random, Pat::Scatter},
        {"Beach Foam",         {1},             50, .2f, .5f, Rot::Random, Pat::Wave},
        {"Beach Sun Rays",     {40},            35, .6f, 1.2f, Rot::Diag45, Pat::Corners},
        {"Beach Palm Dots",    {1, 3},          40, .3f, .6f, Rot::Random, Pat::Scatter},
        // ---- SPACE / CYBER / RETRO ----
        {"Space Dust",         {1},             70, .1f, .2f, Rot::Random, Pat::Scatter},
        {"Space Stars",        {1, 2},          40, .15f, .3f, Rot::Diag45, Pat::Scatter},
        {"Space Rings",        {1},             30, .4f, .7f, Rot::Diag45, Pat::Corners},
        {"Cyber Rain",         {40, 1},         50, .2f, .5f, Rot::None,   Pat::Rain},
        {"Cyber Frame",        {40},            55, .4f, .8f, Rot::Snap90, Pat::Edges},
        {"Retro Arcade",       {1, 2, 3, 4},    45, .3f, .5f, Rot::Snap90, Pat::Grid},
        {"Retro Waves",        {40},            55, .4f, .7f, Rot::None,   Pat::Wave},
        {"Synthwave Stripes",  {40},            50, .8f, 1.5f, Rot::None,  Pat::Rain},
        // ---- MINIMAL / NATURE ----
        {"Minimal Dots",       {1},             20, .15f, .25f, Rot::None, Pat::Edges},
        {"Minimal Lines",      {40},            25, .5f, 1.f, Rot::Snap90, Pat::Edges},
        {"Forest Leaves",      {1, 5},          55, .2f, .5f, Rot::Random, Pat::Scatter},
        {"Forest Rain",        {1},             45, .15f, .3f, Rot::None,  Pat::Rain},
        {"Ocean Bubbles",      {1},             55, .1f, .4f, Rot::Random, Pat::Rain},
        {"Ocean Deep",         {40, 1},         60, .4f, 1.f, Rot::None,   Pat::Wave},
        {"Volcano Sparks",     {1, 2},          60, .1f, .3f, Rot::Diag45, Pat::Scatter},
        {"Ice Crystals",       {1},             40, .2f, .6f, Rot::Diag45, Pat::Corners},
        // ---- FUN / MIX ----
        {"Candy Pop",          {1, 2, 3, 4, 5}, 55, .2f, .5f, Rot::Random, Pat::Scatter},
        {"Dark Tech",          {1, 40},         35, .3f, .7f, Rot::Snap90, Pat::Edges},
        {"Gold Luxury",        {1},             30, .3f, .5f, Rot::Diag45, Pat::Edges},
        {"Neon City",          {40, 1, 3},      55, .3f, 1.f, Rot::Snap90, Pat::Rain},
        {"Glitch Storm",       {1, 40},         70, .1f, 1.f, Rot::Random, Pat::Scatter},
        {"Honeycomb",          {1, 2},          40, .4f, .4f, Rot::Diag45, Pat::Grid},
        {"Mosaic",             {1,2,3,4,5,6,7}, 60, .25f, .25f, Rot::None, Pat::Grid},
        {"Sunset Clouds",      {1, 3},          35, .5f, 1.2f, Rot::None,  Pat::Wave},
        {"Timshadow Style",    {1, 40, 2},      50, .2f, .9f, Rot::Snap90, Pat::Edges},
    };
    return p;
}

static std::vector<int> parseIds(std::string const& s) {
    std::vector<int> out;
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, ',')) {
        try { out.push_back(std::stoi(part)); } catch (...) {}
    }
    return out;
}

class $modify(AutoDecorPause, EditorPauseLayer) {
    struct Fields {
        CCLabelBMFont* label = nullptr;
    };

    bool init(LevelEditorLayer* lel) {
        if (!EditorPauseLayer::init(lel)) return false;

        auto win = CCDirector::get()->getWinSize();
        float cx = win.width - 110.f;

        int idx = Mod::get()->getSavedValue<int>("preset", 0);
        if (idx < 0 || idx >= (int)presets().size()) idx = 0;

        m_fields->label = CCLabelBMFont::create(presets()[idx].name, "bigFont.fnt");
        m_fields->label->limitLabelWidth(120.f, 0.5f, 0.2f);
        m_fields->label->setPosition({cx, 95.f});
        this->addChild(m_fields->label);

        auto menu = CCMenu::create();
        menu->setPosition({cx, 60.f});

        auto left = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        left->setScale(0.6f);
        auto leftBtn = CCMenuItemSpriteExtra::create(
            left, this, menu_selector(AutoDecorPause::onPrev));
        leftBtn->setPosition({-75.f, 25.f});
        menu->addChild(leftBtn);

        auto right = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        right->setFlipX(true);
        right->setScale(0.6f);
        auto rightBtn = CCMenuItemSpriteExtra::create(
            right, this, menu_selector(AutoDecorPause::onNext));
        rightBtn->setPosition({75.f, 25.f});
        menu->addChild(rightBtn);

        auto apply = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Auto Decor"), this,
            menu_selector(AutoDecorPause::onAutoDecor));
        apply->setPosition({0.f, 0.f});
        menu->addChild(apply);

        this->addChild(menu);
        return true;
    }

    void changePreset(int delta) {
        int n = (int)presets().size();
        int i = Mod::get()->getSavedValue<int>("preset", 0);
        i = ((i + delta) % n + n) % n;
        Mod::get()->setSavedValue<int>("preset", i);
        m_fields->label->setString(presets()[i].name);
        m_fields->label->limitLabelWidth(120.f, 0.5f, 0.2f);
    }
    void onPrev(CCObject*) { changePreset(-1); }
    void onNext(CCObject*) { changePreset(1); }

    void onAutoDecor(CCObject*) {
        auto lel = m_editorLayer;
        auto ui = lel->m_editorUI;

        int idx = Mod::get()->getSavedValue<int>("preset", 0);
        if (idx < 0 || idx >= (int)presets().size()) idx = 0;
        Preset const& P = presets()[idx];

        // ID pool = design IDs + custom IDs, minus blocked ones (text, triggers, spikes...)
        std::vector<int> pool;
        for (int id : P.ids) if (!BLOCKED.count(id)) pool.push_back(id);
        for (int id : parseIds(Mod::get()->getSettingValue<std::string>("extra-ids")))
            if (id > 0 && !BLOCKED.count(id)) pool.push_back(id);
        if (pool.empty()) {
            FLAlertLayer::create("Auto Decor", "No valid decor IDs!", "OK")->show();
            return;
        }

        int densityMult = (int)Mod::get()->getSettingValue<int64_t>("density-mult");
        float scaleMult = (float)Mod::get()->getSettingValue<double>("scale-mult");
        int colorCh = (int)Mod::get()->getSettingValue<int64_t>("color-channel");
        int density = std::clamp(P.density * densityMult / 100, 0, 100);

        CCArray* src = ui->getSelectedObjects();
        if (!src || src->count() == 0) src = lel->m_objects;

        std::vector<GameObject*> targets;
        CCObject* o;
        CCARRAY_FOREACH(src, o) {
            if (auto g = typeinfo_cast<GameObject*>(o)) targets.push_back(g);
        }

        std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> pct(1, 100);
        std::uniform_int_distribution<size_t> pick(0, pool.size() - 1);
        std::uniform_real_distribution<float> scaleD(P.minS, P.maxS);
        std::uniform_real_distribution<float> rot360(0.f, 360.f);
        std::uniform_real_distribution<float> ang(0.f, 6.2831853f);
        std::uniform_real_distribution<float> radius(25.f, 80.f);
        float jAmt = (P.pat == Pat::Grid) ? 0.f : 6.f;
        std::uniform_real_distribution<float> jit(-jAmt - 0.001f, jAmt + 0.001f);

        const int MAX_CREATE = 8000;
        int created = 0;

        for (auto g : targets) {
            if (created >= MAX_CREATE) break;
            CCPoint c = g->getPosition();
            std::vector<CCPoint> pts;

            switch (P.pat) {
                case Pat::Edges:
                    pts = {{c.x, c.y + 30}, {c.x, c.y - 30}, {c.x - 30, c.y}, {c.x + 30, c.y}};
                    break;
                case Pat::Corners:
                    pts = {{c.x - 30, c.y + 30}, {c.x + 30, c.y + 30},
                           {c.x - 30, c.y - 30}, {c.x + 30, c.y - 30}};
                    break;
                case Pat::Scatter:
                    for (int k = 0; k < 3; k++) {
                        float a = ang(rng), r = radius(rng);
                        pts.push_back({c.x + std::cos(a) * r, c.y + std::sin(a) * r});
                    }
                    break;
                case Pat::Rain:
                    for (int k = 1; k <= 4; k++) pts.push_back({c.x, c.y + 30.f * k});
                    break;
                case Pat::Wave:
                    for (int k = -2; k <= 2; k++) {
                        float x = c.x + 30.f * k;
                        pts.push_back({x, c.y + 40.f + std::sin(x * 0.04f) * 12.f});
                    }
                    break;
                case Pat::Grid:
                    for (int dx = -1; dx <= 1; dx++)
                        for (int dy = -1; dy <= 1; dy++)
                            if (dx || dy) pts.push_back({c.x + dx * 30.f, c.y + dy * 30.f});
                    break;
            }

            for (auto p : pts) {
                if (pct(rng) > density) continue;
                p.x += jit(rng);
                p.y += jit(rng);

                auto obj = lel->createObject(pool[pick(rng)], p, false);
                if (!obj) continue;

                float s = scaleD(rng) * scaleMult;
                obj->updateCustomScaleX(s);
                obj->updateCustomScaleY(s);

                switch (P.rot) {
                    case Rot::None: break;
                    case Rot::Random: obj->setRotation(rot360(rng)); break;
                    case Rot::Snap90: obj->setRotation(90.f * (rng() % 4)); break;
                    case Rot::Diag45: obj->setRotation(45.f); break;
                }

                if (colorCh > 0 && obj->m_baseColor) {
                    obj->m_baseColor->m_colorID = colorCh;
                }
                created++;
                if (created >= MAX_CREATE) break;
            }
        }

        FLAlertLayer::create(
            "Auto Decor",
            fmt::format("Design: {}\nObjects created: {}", P.name, created),
            "OK"
        )->show();
    }
};
