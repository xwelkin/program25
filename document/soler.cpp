#include "Soler.h"
using namespace std;
namespace Color{//定义color命名空间，用于给打印的文本添加颜色
    //基础色值
    inline constexpr const char* Reset = "\x1b[0m";
    inline constexpr const char* White = "\x1b[37m";
    inline constexpr const char* Red = "\x1b[31m";
    inline constexpr const char* Green = "\x1b[32m";
    inline constexpr const char* Yellow = "\x1b[33m";//亚里德尔
    inline constexpr const char* Blue = "\x1b[34m";//围观者/民众
    inline constexpr const char* Magenta = "\x1b[35m";//贵族阵营
    inline constexpr const char* Cyan = "\x1b[36m";//猎人
    inline constexpr const char* BrightBlack = "\x1b[90m";
    inline constexpr const char* BrightRed = "\x1b[91m";
    inline constexpr const char* BrightGreen = "\x1b[92m";
    inline constexpr const char* BrightYellow = "\x1b[93m";
    inline constexpr const char* BrightBlue = "\x1b[94m";
    inline constexpr const char* BrightMagenta = "\x1b[95m";// 粉色（涅索）
    inline constexpr const char* BrightCyan = "\x1b[96m";
    inline constexpr const char* BrightWhite = "\x1b[97m";
}

//说话人标识，不直接添加到文本中的原因是后期会根据对话内容添加不同的颜色
enum class Speaker{
    Narrator,//旁白（白）
    Solervia,//索尔维娅（橙）
    Neso,//涅索（粉）
    Cato,//卡托（红）
    Hunter,//猎人（青）
    RomanCrowd,//围观的罗马人/观众/民众（蓝）
    Noble,//贵族阵营（紫）
    Gladiator,//角斗士/陌生角斗士（红）
    Aridel,//亚里德尔（黄）
    Other
};

//简易颜色/前缀适配（仅声明，实现在后续实现阶段补全）
struct StyledPrinter{
    static const char* colorOf(Speaker who);
    static const char* nameOf(Speaker who);
};

//玩家属性(这边觉得结构体就够了，后续的战斗里面，主角只起到一个显示名字的作用就行，其余还在于技能的实现)
struct PlayerStats{
    int hp = 100;//生命
    int martial = 5;//武艺
    int will = 5;//意志
    int prestige = 5;//威望
    int luck = 5;//运气

    void clampNonNegative();//用于防止属性值为负数
};

//选项结构体
struct Choice{
    string text;//展示文本（不含数值变化的提示）
    function<void(PlayerStats&)> apply;//执行时对属性的修改
    int nextScene = 1;//默认进入下一段；可设为其它偏移以跳转
    bool leadsBadEnd = false;//触发坏结局
    string badEndDescription;//坏结局描述（若触发）

    //这里使用了结构体的构造器：允许只给前面若干参数，其他使用默认值，避免缺失初始化警告
    Choice() = default;
    Choice(string t,
           function<void(PlayerStats&)> a = {},
           int offset = 1,
           bool bad = false,
           string desc = "")
        : text(move(t)), apply(move(a)), nextScene(offset), leadsBadEnd(bad), badEndDescription(move(desc)) {}
};

//段落/场景结构体
struct Scene{
    Speaker speaker = Speaker::Narrator; //谁在说话（用于配色）
    string content;//文案（与 md 一致）
    Choice choice1;//选项1
    Choice choice2;//选项2
    bool entersBattle = false;//是否进入战斗（仅流程占位）
};

//幕（由多个 Scene 组成）
struct Act {
    string name;//幕名
    vector<Scene> scenes;//顺序执行
    int prestigeThreshold = -1;//幕末威望门槛（-1 表示无）
};
/**************************************************
 * 这里只是占位，如果你们想使用类来书写也可以改为类，具体内容请自己添加，还需要设定一个菜单，
 * 显示当前的属性，装备，技能，背包，商店，战斗等信息，也可以选择退出游戏
 **************************************************/
//战斗/背包/商店/技能等占位
struct Battle { };//战斗
struct Inventory { };//背包
struct Merchant { };//商人
struct Skill { };//技能

//坏结局处理返回的动作
enum class BadEndAction{
    RestartAct,//重新开始当前幕
    RestartGame,//重新开始游戏
    Quit//退出游戏
};

//剧情引擎（仅声明）
class StoryEngine{
public:
    StoryEngine();//构造函数:加载全部幕与场景定义

    void resetToPrologue();//重置到序幕并初始化玩家属性
    void startGameLoop();//启动游戏主循环(前言→横幅→运行当前幕)

private:
    //基础交互
    int askBinaryChoice(const Choice& c1, const Choice& c2);//打印两个选项并读取1/2(非法重试)
    void printLine(Speaker who, const string& text);//按说话人颜色打印一行文本
    void applyChoiceAndAdvance(Choice& choice);//应用选项修改并打印属性变化
    void printDelta(const PlayerStats& before, const PlayerStats& after);//比较前后属性并输出变化提示
    BadEndAction handleBadEnd(const string& description);//显示坏结局与菜单,返回玩家选择
    //幕控制
    void loadActs();//构建序幕与四幕的全部场景
    void runAct(size_t actIndex);//运行指定幕,处理场景/分支/战斗/门槛/幕间
    bool checkPrestigeGate(const Act& act) const;//检查本幕威望门槛(≥threshold)
    //战斗占位
    void runBattlePlaceholder(const string& title,
                              bool equipmentAllowed);//战斗占位,仅打印"进入战斗!"
    //幕间占位
    void interludeFirst();//第一幕与第二幕之间幕间:与卡托对话,运气+10
    void interludeShop();//第二、三次幕间入口占位(商店占位)
    void printStarBanner();//游戏开场横幅
    void printPrefaceAndWait();//开场黄字前言与“输入1继续”

private:
    vector<Act> acts;
    size_t currentActIndex = 0;
    PlayerStats player;
    //幕起点快照（失败回到本幕第一段时恢复，在此时显示保存成功，为上一复活节点）
    PlayerStats actEntrySnapshot;
};
//颜色设定
const char* StyledPrinter::colorOf(Speaker who) {
    switch (who) {
        case Speaker::Narrator: return Color::White;
        case Speaker::Solervia: return "\x1b[38;5;208m";//与橙色近似
        case Speaker::Neso: return Color::BrightMagenta;//粉色
        case Speaker::Cato: return Color::Red;//红色
        case Speaker::Hunter: return Color::Cyan;//青色
        case Speaker::RomanCrowd: return Color::Blue;//蓝色
        case Speaker::Noble: return Color::Magenta;//紫色
        case Speaker::Gladiator: return Color::Red;//红色
        case Speaker::Aridel: return Color::Yellow;//黄色
        default: return Color::White;//默认白色
    }
}
//中文名设定
const char* StyledPrinter::nameOf(Speaker who) {
    switch (who) {
        case Speaker::Narrator: return "旁白";
        case Speaker::Solervia: return "索尔维娅";
        case Speaker::Neso: return "涅索";
        case Speaker::Cato: return "卡托";
        case Speaker::Hunter: return "猎人";
        case Speaker::RomanCrowd: return "民众";
        case Speaker::Noble: return "贵族";
        case Speaker::Gladiator: return "角斗士";
        case Speaker::Aridel: return "亚里德尔";
        default: return "";//默认空字符串
    }
}
//属性最小值为0，无最大值，防止负数
void PlayerStats::clampNonNegative() {
    auto clamp0 = [](int& v) { if (v < 0) v = 0; };
    clamp0(hp);
    clamp0(martial);
    clamp0(will);
    clamp0(prestige);
    clamp0(luck);
}

StoryEngine::StoryEngine() {
    loadActs();
}

void StoryEngine::resetToPrologue() {
    currentActIndex = 0;
    player = PlayerStats{};
    actEntrySnapshot = player;
}

void StoryEngine::startGameLoop() {
    if (acts.empty()) return;
    printPrefaceAndWait();
    printStarBanner();
    runAct(currentActIndex);
}

int StoryEngine::askBinaryChoice(const Choice& c1, const Choice& c2) {
    cout << Color::White << c1.text << Color::Reset << "\n";
    cout << Color::White << c2.text << Color::Reset << "\n";
    cout << Color::BrightBlack << "请输入 1 或 2 并回车：" << Color::Reset;
    cout.flush();
    string in;
    while (true) {
        if (!getline(cin, in)) return 1;
        if (in == "1" || in == "2") break;
        cout << Color::BrightBlack << "无效输入，请输入 1 或 2：" << Color::Reset;
        cout.flush();
    }
    return in == "1" ? 1 : 2;
}

void StoryEngine::printLine(Speaker who, const string& text) {
    cout << StyledPrinter::colorOf(who) << text << Color::Reset << "\n";
}

void StoryEngine::applyChoiceAndAdvance(Choice& choice) {
    PlayerStats before = player;
    if (choice.apply) {
        choice.apply(player);
        player.clampNonNegative();
    }
    printDelta(before, player);
}
//
void StoryEngine::printDelta(const PlayerStats& before, const PlayerStats& after) {
    struct Item { const char* name; int delta; };
    vector<Item> items = {
        {"生命", after.hp - before.hp},
        {"武艺", after.martial - before.martial},
        {"意志", after.will - before.will},
        {"威望", after.prestige - before.prestige},
        {"运气", after.luck - before.luck}
    };
    bool any = false;
    string line = "属性变化：";
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].delta != 0) {
            any = true;
            string part = string(items[i].name) + (items[i].delta > 0 ? "+" : "") + to_string(items[i].delta);
            line += part + " ";
        }
    }
    if (any) {
        cout << Color::BrightGreen << line << Color::Reset << "\n";
    }
}

BadEndAction StoryEngine::handleBadEnd(const string& description) {
    printLine(Speaker::Narrator, description);
    Choice c1{ "1.从上一个存档点继续", {} };
    Choice c2{ "2.重新开始游戏", {} };
    cout << Color::White << c1.text << Color::Reset << "\n";
    cout << Color::White << c2.text << Color::Reset << "\n";
    cout << Color::White << "3.退出游戏" << Color::Reset << "\n";
    cout << Color::BrightBlack << "请输入 1/2/3：" << Color::Reset;
    cout.flush();
    string in;
    while (true) {
        if (!getline(cin, in)) return BadEndAction::Quit;
        if (in == "1") return BadEndAction::RestartAct;
        if (in == "2") return BadEndAction::RestartGame;
        if (in == "3") return BadEndAction::Quit;
        cout << Color::BrightBlack << "无效输入，请输入 1/2/3：" << Color::Reset;
        cout.flush();
    }
}
//检查威望门槛
bool StoryEngine::checkPrestigeGate(const Act& act) const {
    if (act.prestigeThreshold < 0) return true;
    return player.prestige >= act.prestigeThreshold;
}
//战斗占位
void StoryEngine::runBattlePlaceholder(const string& title, bool equipmentAllowed) {
    (void)title;//抑制未使用参数告警
    (void)equipmentAllowed;//抑制未使用参数告警
    printLine(Speaker::Narrator, "进入战斗！");
}
//第一幕与第二幕之间的幕间
void StoryEngine::interludeFirst() {
    printLine(Speaker::Cato, "明天训练的时间，六点，就在这里......");
    player.luck += 10;
    player.clampNonNegative();
    printLine(Speaker::Narrator, "你与卡托交谈，获得了好运的眷顾（运气+10）。");
}
//第二、三幕结束后的幕间（商店占位）
void StoryEngine::interludeShop() {
    printLine(Speaker::Narrator, "你可以与商人交易（占位功能，当前未实现）。\n1.返回\n2.继续");
    string in;
    cout.flush();
    while (true) {
        if (!getline(cin, in)) break;
        if (in == "1" || in == "2") break;
        cout << Color::BrightBlack << "无效输入，请输入 1 或 2：" << Color::Reset;
        cout.flush();
    }
}
//幕控制
void StoryEngine::runAct(size_t actIndex) {
    if (actIndex >= acts.size()) return;
    currentActIndex = actIndex;
    Act& act = acts[actIndex];
    actEntrySnapshot = player;//存档
    //进入每一幕前，提示存档成功（绿色），随后打印幕标题（黄色）
    cout << Color::BrightGreen << "存档成功" << Color::Reset << "\n";
    cout << Color::Yellow << act.name << Color::Reset << "\n";
    //标题下一行打印名望要求
    {
        string req = "0";
        if (actIndex == 1) {
            req = "> 90";
        } else if (act.prestigeThreshold >= 0) {
            req = string("≥ ") + to_string(act.prestigeThreshold);
        }
        printLine(Speaker::Narrator, string("名望要求：达到") + req);
    }

    size_t i = 0;
    while (i < act.scenes.size()) {
        Scene& s = act.scenes[i];
        printLine(s.speaker, s.content);
        int chosen = askBinaryChoice(s.choice1, s.choice2);
        Choice* chosenPtr = (chosen == 1) ? &s.choice1 : &s.choice2;

        if (chosenPtr->leadsBadEnd) {
            BadEndAction action = handleBadEnd(chosenPtr->badEndDescription.empty() ? "你选择了错误的道路，故事在此刻戛然而止。" : chosenPtr->badEndDescription);//处理了目前没有文本的坏结局
            if (action == BadEndAction::RestartAct) {
                player = actEntrySnapshot;
                i = 0;
                continue;
            } else if (action == BadEndAction::RestartGame) {
                resetToPrologue();
                runAct(0);
                return;
            } else {
                return;
            }
        }

        applyChoiceAndAdvance(*chosenPtr);

        //选择后自动判定：生命=0 或 意志=0 → 进入坏结局（威望=0不触发）
        if (player.hp == 0) {
            BadEndAction action = handleBadEnd("旁白：你的身体已到达极限，无法支撑你继续走接下来的路。（生命或意志为 0）。");
            if (action == BadEndAction::RestartAct) {
                player = actEntrySnapshot;
                i = 0;
                continue;
            } else if (action == BadEndAction::RestartGame) {
                resetToPrologue();
                runAct(0);
                return;
            } else {
                return;
            }
        }
        if (player.will == 0) {
            BadEndAction action = handleBadEnd("旁白：你的意志被击垮，你已经失去了所有的热情。（生命或意志为 0）。");
            if (action == BadEndAction::RestartAct) {
                player = actEntrySnapshot;
                i = 0;
                continue;
            } else if (action == BadEndAction::RestartGame) {
                resetToPrologue();
                runAct(0);
                return;
            } else {
                return;
            }
        }
        if (s.entersBattle) {
            runBattlePlaceholder(act.name + " 战斗", true);
        }

        int offset = chosenPtr->nextScene;
        if (offset <= 0) offset = 1;
        i += static_cast<size_t>(offset);
    }

    //第一幕末门槛特殊：>90
    if (actIndex == 1) {
        if (!(player.prestige > 90)) {
            BadEndAction action = handleBadEnd("旁白：你的威望尚不足以撼动众人的偏见。（需要>90）。");
            if (action == BadEndAction::RestartAct) {
                player = actEntrySnapshot; runAct(actIndex); return;
            } else if (action == BadEndAction::RestartGame) {
                resetToPrologue(); runAct(0); return;
            } else { return; }
        }
        interludeFirst();//第一幕与第二幕之间的幕间
    } else if (acts[actIndex].prestigeThreshold >= 0) {
        if (!checkPrestigeGate(acts[actIndex])) {
            string gateMsg = "旁白：你的威望未达门槛，故事在此终结。";
            if (actIndex == 2) gateMsg = "旁白：你的威望尚未达到进入第三幕的要求（需要≥130）。";
            else if (actIndex == 3) gateMsg = "旁白：你的威望尚未达到进入第四幕的要求（需要≥240）。";
            BadEndAction action = handleBadEnd(gateMsg);
            if (action == BadEndAction::RestartAct) {
                player = actEntrySnapshot; runAct(actIndex); return;
            } else if (action == BadEndAction::RestartGame) {
                resetToPrologue(); runAct(0); return;
            } else { return; }
        }
        //第二、三幕结束后的幕间（商店占位）
        if (actIndex == 2 || actIndex == 3) {
            interludeShop();
        }
    }

    if (actIndex + 1 < acts.size()) runAct(actIndex + 1);
}

void StoryEngine::loadActs() {
    acts.clear();

    Act prologue;
    prologue.name = "烈日昔叙之诗 · 序幕";
    prologue.prestigeThreshold = -1;

    {
        Scene s;
        s.speaker = Speaker::Neso;
        s.content = "？？？：远道而来的客人，请随我步入烈日少女那快要被遗忘的旧事，不过如果仅听他人口述，或许这个故事会稍显无聊。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        prologue.scenes.push_back(s);
    }
    {
        Scene s;
        s.speaker = Speaker::Neso;
        s.content = "？？？：也许，让倾听者成为这段往事的主人公是个不错的选择。如此一来，那些你不曾听见的声音、不曾目睹的画面便能“触手可及”";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        prologue.scenes.push_back(s);
    }
    {
        Scene s;
        s.speaker = Speaker::Neso;
        s.content = "？？？：原谅我可能让您不适的热情，还未来得及自我介绍。流浪诗人，涅索。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        prologue.scenes.push_back(s);
    }
    {
        Scene s;
        s.speaker = Speaker::Neso;
        s.content = "涅索：此时此刻，你即是“烈日少女”，她的真名叫做索尔维娅。你将亲身经历她的过去，亦会重新面临她过往的抉择。所以......请暂时忘却你的名字。";
        s.choice1 = { "1.来吧。", {} };
        s.choice2 = { "2.角色扮演吗？", {} };
        prologue.scenes.push_back(s);
    }
    {
        Scene s;
        s.speaker = Speaker::Neso;
        s.content = "涅索：当然，关于索尔维娅的篇章早已写就。不论你做出何种选择，现实皆不会因此而改变。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        prologue.scenes.push_back(s);
    }
    {
        Scene s;
        s.speaker = Speaker::Neso;
        s.content = "涅索：然而我们仍然需要格外留心，命运常会因为某些意外而偏离既定的轨迹，戛然而止。所以在面对选择时，请务必谨慎考虑。";
        s.choice1 = { "1.过去的选择也不一定是最好的吧。", {} };
        s.choice2 = { "2.我知道了。", {} };
        prologue.scenes.push_back(s);
    }
    {
        Scene s;
        s.speaker = Speaker::Neso;
        s.content = "涅索：那么，请让这帷幕拉开。放下繁杂的思绪，听我为你唱诵一位少女——索尔维娅的诗篇。";
        s.choice1 = { "1.（暗示自己是索尔维娅）", {} };
        s.choice2 = { "2.（深呼吸）", {} };
        prologue.scenes.push_back(s);
    }

    acts.push_back(move(prologue));

    //Act 1: 第一幕（严格>90威望门槛在幕末判定，这里门槛字段保持-1）
    Act act1;
    act1.name = "烈日昔叙之诗 · 第一幕";
    act1.prestigeThreshold = -1;

    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：现在，温暖的光模糊了你的视野，你缓缓垂落眼帘......等再次睁眼时，一位身形单薄的少女紧握着剑，如惊弓之鸟般站在破败的土地上。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "几位角斗士肃然守护在她的身边。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Gladiator;
        s.content = "陌生角斗士：孩子，只有你一个人吗？别害怕，我们知道战争毁灭了你的故乡，我们是来帮助你的。";
        s.choice1 = { "1.胆怯地打量眼前的人。", {} };
        s.choice2 = { "2.大胆地打量眼前的人。", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Gladiator;
        s.content = "陌生角斗士：你一定饿坏了吧，给......这些食物你先拿着吧。";
        s.choice1 = { "1.拒绝角斗士递过来的食物。", [](PlayerStats& p){ p.hp -= 10; } };
        s.choice2 = { "2.接过角斗士递过来的食物。", [](PlayerStats& p){ p.hp += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Gladiator;
        s.content = "陌生角斗士：孩子，我可以带你去罗马城邦。下一场灾难不知道什么时候会再次降临，你一直看着这些残垣断壁也没有任何意义了。";
        s.choice1 = { "1.（继续）", [](PlayerStats& p){ p.martial += 10; } };
        s.choice2 = { "2.（继续）", [](PlayerStats& p){ p.martial += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Gladiator;
        s.content = "陌生角斗士：就算你将这些墙壁修缮千万遍，在战争面前也是渺小无力的。你根本没有力量抵抗它，带上你的剑跟我走吧......";
        Choice c1{ "1.麻木地松开剑。", {}, 1, true };
        c1.badEndDescription = "旁白：你放下了剑，与守护的意志一同坠落。你的故事在此处终结。";
        Choice c2{ "2.坚毅地握着剑。", [](PlayerStats& p){ p.will += 20; } };
        s.choice1 = c1; s.choice2 = c2;
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：角斗士们摇摇头，拍了拍你的肩膀。";
        s.choice1 = { "1.犹豫地看了看残破的小镇。", [](PlayerStats& p){ p.will -= 5; } };
        s.choice2 = { "2.决定跟上他们。", [](PlayerStats& p){ p.prestige += 20; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你最终还是选择离开故乡。你被角斗士的话触动，认为自己根本没有力量保护身后的土地。一路上，你......";
        s.choice1 = { "1.一个人安静地待着。", [](PlayerStats& p){ p.prestige -= 10; } };
        s.choice2 = { "2.与角斗士积极交谈。", [](PlayerStats& p){ p.prestige += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你跟随角斗士乘船抵达了罗马城邦。在城中心的广场上，你与角斗士们挥手作别。与此同时，你注意到此处聚集着大量罗马人。";
        s.choice1 = { "1.对城区的一切感到不安。", [](PlayerStats& p){ p.will -= 5; } };
        s.choice2 = { "2.对城区的一切感到新鲜。", [](PlayerStats& p){ p.will += 20; p.prestige += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：一张角斗士竞技的海报被牢牢地钉在木板上，众人正在热议关于比赛的话题。";
        s.choice1 = { "1.被推搡着走进人群。", [](PlayerStats& p){ p.hp -= 5; } };
        s.choice2 = { "2.主动走进人群。", [](PlayerStats& p){ p.prestige += 10; p.hp += 20; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "围观的罗马人：要知道，我们罗马向来崇尚力量，胜者为王是这里亘古不变的法则。";
        s.choice1 = { "1.看到了这位罗马人眼中对弱者的冷漠。", [](PlayerStats& p){ p.will -= 5; } };
        s.choice2 = { "2.看到了这位罗马人眼中的兴奋。", [](PlayerStats& p){ p.prestige += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：要在这弱肉强食的夹缝中生存下来，唯有用自己的獠牙咬下那鼎立于万人之上的冠冕。";
        s.choice1 = { "1.（继续）", [](PlayerStats& p){ p.hp += 20; } };
        s.choice2 = { "2.（继续）", [](PlayerStats& p){ p.hp += 20; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：他们的讨论声在你的脑海中挥之不去，你想到角斗士在罗马被视为力量的象征。若是自己能登上竞技场之巅的话，或者就能找到你所需要的力量......";
        s.choice1 = { "1.撕下海报，离开人群。", [](PlayerStats& p){ p.hp -= 10; p.will += 10; p.prestige += 5; } };
        s.choice2 = { "2.默默离开人群。", [](PlayerStats& p){ p.will += 5; p.hp += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "围观的罗马人：总有些俗人将角斗竞技当做一场无聊的杀戮表演，可惜我们没兴趣看老鼠在尘埃中嘶叫。我们要的，是让玫瑰在血中绽放的华丽之人。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "冷漠的罗马人：打个赌吧，今年又会有多少个不自量力的鼠辈登上竞技台，带着他们那股令人作呕的腐臭味。";
        s.choice1 = { "1.有些忧虑自己的地位、天赋。", [](PlayerStats& p){ p.will -= 10; } };
        s.choice2 = { "2.无视他们的言论。", [](PlayerStats& p){ p.will += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：离开人群前，你关注到身份、地位以及天赋频繁出现在人们的议论中。看来这些因素在角斗竞技中至关重要。";
        s.choice1 = { "1.（继续）", [](PlayerStats& p){ p.hp += 20; } };
        s.choice2 = { "2.（继续）", [](PlayerStats& p){ p.hp += 20; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：多次碰壁后，你找到了一位因伤隐退的角斗士，卡托。他曾是罗马过去的明星角斗士，招收的学生寥寥无几。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：呵，想参加角斗竞技吗？先介绍一下你自己吧。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你回想起那些关于角斗士的言论——天赋、身份、地位，城里的罗马人似乎更看重这些东西，你决定......";
        s.choice1 = { "1.（编造一个来历）", [](PlayerStats& p){ p.prestige += 5; } };
        s.choice2 = { "2.认真诉说自己的来历。", [](PlayerStats& p){ p.prestige += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：故乡被战争毁灭，为了寻求力量来到罗马城邦吗？你很有勇气，可是光靠勇气并不能改变什么。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：回去吧，总会有像你这样一无所有的人来到此地，抱着遥不可及的理想自我感动，然后轻而易举地被现实碾碎。我受够了这样的蠢货。";
        s.choice1 = { "1.沉默。", [](PlayerStats& p){ p.will -= 10; p.prestige -= 10; } };
        s.choice2 = { "2.试图反驳。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：这样吧，你和我比一场，只要刺中我一剑就算你赢。赢了之后，我会教你角斗士真正的战斗技巧。这场比试，就是你的入门考试。";
        s.choice1 = { "1.立即接受比试。", [](PlayerStats& p){ p.martial -= 5; p.will += 10; } };
        s.choice2 = { "2.仔细思考后，接受比试。", [](PlayerStats& p){ p.martial += 10; p.will += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：刀剑碰撞声随之响起，卡托看起来非常游刃有余。你很快便被他击败。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：毫无力量可言，你现在明白自己有多弱小了吧。";
        s.choice1 = { "1.正因如此，我才想成为角斗士。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; p.hp += 20; } };
        s.choice2 = { "2.正因如此，我才寻求力量。", [](PlayerStats& p){ p.will += 10; p.martial += 10; p.hp += 20; } };
        act1.scenes.push_back(s);
    }

    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：怎么？还不走？是准备用你那套自我感动来打动我吗？无知者真是可悲。";
        s.choice1 = { "1.握紧手中的剑。", {} };
        s.choice2 = { "2.直视卡托。", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：呵，你清楚你的对手是谁吗？你真的能赢得了我吗？赢不了，你打算怎么收场。";
        s.choice1 = { "1.第二天继续挑战。", [](PlayerStats& p){ p.martial += 10; p.will += 5; } };
        s.choice2 = { "2.就此放弃。", {}, 1, true, "旁白：放弃使你一蹶不振，你再也没有机会成为角斗士，唯有仇恨伴随一生。" };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：哦？又来了。按城里大多数罗马人的说法，你们这些乡下来的老鼠可真够缠人的。";
        s.choice1 = { "1.防御。", [](PlayerStats& p){ p.martial -= 10; p.hp += 10; } };
        s.choice2 = { "2.进攻。", [](PlayerStats& p){ p.martial += 10; p.hp -= 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：卡托依旧游刃有余，你的力量在他面前不堪一击。你很快又被他击败了。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：回去吧，这不是你要走的路......";
        s.choice1 = { "1.第三天继续挑战。", [](PlayerStats& p){ p.martial += 5; p.will += 10; p.hp += 10; } };
        s.choice2 = { "2.就此放弃。", {}, 1, true, "旁白：放弃使你一蹶不振，你再也没有机会成为角斗士，唯有仇恨伴随一生。" };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：速度不够，力量也不够......你那幼稚的剑术根本威胁不到敌人。";
        s.choice1 = { "1.那便更强。", [](PlayerStats& p){ p.martial += 10; p.prestige += 10; p.hp += 10; } };
        s.choice2 = { "2.那就更快。", [](PlayerStats& p){ p.martial += 20; p.prestige += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：就这点能耐？太天真了。阴影里的老鼠注定见不得光。胜负已分，你输了。";
        s.choice1 = { "1.第四天继续挑战。", [](PlayerStats& p){ p.martial += 5; p.will += 10; p.hp += 10; } };
        s.choice2 = { "2.就此放弃。", {}, 1, true, "旁白：放弃使你一蹶不振，你再也没有机会成为角斗士，唯有仇恨伴随一生。" };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：还不准备放弃吗？不如把你的剑卖了，多少还能换点吃的。";
        s.choice1 = { "1.第五天，继续挑战。", [](PlayerStats& p){ p.martial += 5; p.will += 10; p.hp += 10; } };
        s.choice2 = { "2.就此放弃。", {}, 1, true, "旁白：放弃使你一蹶不振，你再也没有机会成为角斗士，唯有仇恨伴随一生。" };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你拖着疲惫的身体再次站到卡托面前，身体已经濒临极限。";
        s.choice1 = { "1.我还能坚持......", [](PlayerStats& p){ p.will += 5; p.prestige += 10; p.hp += 10; } };
        s.choice2 = { "2.就此放弃", {}, 1, true, "旁白：放弃使你一蹶不振，你再也没有机会成为角斗士，唯有仇恨伴随一生。" };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：在剑风掠过的瞬间，你仿佛回到了久违的梦乡，柔和的阳光洒在大橡树繁茂的枝叶上，你听见伙伴们的欢笑声和父母的温柔呼唤。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：然而下一刻，一股刺入骨髓的寒意扑面而来将你吞没，这种感觉你在熟悉不过。思绪在脑海中轰然炸响，你的手已本能地握紧了剑柄。";
        s.choice1 = { "1.还不可以认输......", [](PlayerStats& p){ p.will += 10; p.prestige += 10; p.hp += 10; } };
        s.choice2 = { "2.还不可以在这里倒下......", [](PlayerStats& p){ p.will += 5; p.prestige += 5; p.hp += 20; } };
        s.entersBattle = true; //此处才进入与卡托的战斗
        act1.scenes.push_back(s);
    }

    //战后奖励：武艺+10
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你抓住破绽，击中了卡托。他将自己的剑递给了你后，转身离开……";
        s.choice1 = { "1.叫住卡托。", [](PlayerStats& p){ p.will -= 10; p.prestige += 10; p.martial += 10; } };
        s.choice2 = { "2.接住卡托的剑，他的剑很重。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; p.martial += 10; } };
        act1.scenes.push_back(s);
    }

    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：在罗马的竞技场上，胜者能够收走败者的武器，象征力量的征服，这把剑给你了......";
        s.choice1 = { "1.征服......", [](PlayerStats& p){ p.will += 10; p.prestige += 5; } };
        s.choice2 = { "2.胜者......", [](PlayerStats& p){ p.will += 5; p.prestige += 5; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：既然你决心要咬下那顶冠冕，就不要轻易地在中途倒下。你的名字叫索尔维娅对吧？";
        s.choice1 = { "1.是的，我叫索尔维娅。", [](PlayerStats& p){ p.prestige += 10; } };
        s.choice2 = { "2.点头。", [](PlayerStats& p){ p.prestige += 5; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：从踏入竞技场的那一刻去，你将失去自己的名字。那些看台上的人会以编号、字母，甚至是野兽的名号来称呼你。";
        s.choice1 = { "1.我别无选择", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        s.choice2 = { "2.我做好了准备", [](PlayerStats& p){ p.prestige += 10; p.hp += 10; } };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：只有站到最后的胜者才有资格向世人高呼自己的真名。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：到那时，众人高呼你的名字——索尔维娅。你将无视天赋、地位、身份，征服所有人心，赢得所有掌声。你说那一天，我能看见吗？";
        s.choice1 = { "1.我会让众人知晓我的真名。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        s.choice2 = { "2.我会让众人的欢呼声响起。", [](PlayerStats& p){ p.prestige += 10; } };
        act1.scenes.push_back(s);
    }
    //第一幕结束语与进入幕间
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：明天训练的时间，六点，就在这里......";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act1.scenes.push_back(s);
    }
    acts.push_back(move(act1));
    //Act 2: 第二幕（有威望门槛>=130，将在幕末按阈值检查）
    Act act2;
    act2.name = "烈日昔叙之诗 · 第二幕 角斗士训练";
    act2.prestigeThreshold = 130;

    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：天色微亮，你在卡托的指导下度过了整整一个月，而今天，训练仍要继续。";
        s.choice1 = { "1.偶尔偷偷懒吧，晚点去。", [](PlayerStats& p){ p.hp += 10; p.martial += 5; p.will -= 10; } };
        s.choice2 = { "2.按时训练。", [](PlayerStats& p){ p.martial += 5; p.will += 10; p.hp += 20; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你来到训练场。你想起现在的这些招式你已练习了无数次，耐心正被一点点消磨。你瞥见几位角斗士正在学习你从未尝试过的技巧，你决定......";
        s.choice1 = { "1.偷看他人练习的技巧。", [](PlayerStats& p){ p.martial += 10; p.will += 5; p.prestige += 5; } };
        s.choice2 = { "2.集中一点，耐心训练。", [](PlayerStats& p){ p.martial += 5; p.will += 10; p.prestige += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你回想起训练的生活，自始至终，你都是一个人前行，一个人面对木桩，一个人为自己擦拭伤药。";
        s.choice1 = { "1.难过。", [](PlayerStats& p){ p.hp += 10; p.will -= 10; } };
        s.choice2 = { "2.忍耐。", [](PlayerStats& p){ p.hp += 20; p.will += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：我能教给你的东西都教了，剩下的路要靠你自己走了。";
        s.choice1 = { "1.今天是毕业仪式？", {} };
        s.choice2 = { "2.训练要结束了吗？", {} };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：这是我留给你最后的“课堂作业”，告诉我，你为什么要成为角斗士？你忍受着这些训练，究竟是为了什么？";
        s.choice1 = { "1.不知道该如何回答。", {} };
        s.choice2 = { "2.内心早已有了答案。", [](PlayerStats& p){ p.will += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你从未忘记离开故乡时的理想——寻求守护他人的力量。于是，你毫不畏惧地迎上了卡托的目光，而他也从你的眼中读出了答案。";
        s.choice1 = { "1.向卡托表示谢意。", [](PlayerStats& p){ p.will += 5; p.prestige += 10; } };
        s.choice2 = { "2.我铭记着自己的理想。", [](PlayerStats& p){ p.will += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：到那时，众人高呼你的名字——奥古斯塔。你将无视天赋、地位、身份，征服所有人心，赢得所有掌声。你说那一天，我能看见吗？";
        s.choice1 = { "1.我已经开始期待那副光景。", {} };
        s.choice2 = { "2.我会做到的。", {} };
        act2.scenes.push_back(s);
    }

    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你知道角斗士比赛的引荐人大多数是贵族和富商，平日你根本无缘接触到他们，除了在地下竞技场……";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "地下商人：嘿嘿，这一批都是“上等货”，各位老爷们要不要看看？";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你和其他角斗士如商品一般站在地下竞技场中央，被观众席上的贵族和富商们打量着。你看着身旁的人一个个被选中，然后离开……";
        s.choice1 = { "1.自我推荐。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        s.choice2 = { "2.向前一步。", [](PlayerStats& p){ p.will += 5; p.prestige += 5; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：观众席上的目光纷纷投向你。你的主动换来的，却是一阵哄笑。";
        s.choice1 = { "1.脑海中闪过贵族的奢靡生活。", [](PlayerStats& p){ p.hp += 10; p.will -= 10; } };
        s.choice2 = { "2.故乡的惨剧在脑海里浮现。", [](PlayerStats& p){ p.hp -= 10; p.will += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：这些嘲笑声……果然，那场悲剧背后，也有你们的影子……";
        s.choice1 = { "1.故乡的悲剧也来源于贵族的腐败。", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.故乡的悲剧不仅仅是战争造成的。", [](PlayerStats& p){ p.will += 5; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：当战争来临时，手握财富和权力之人仍在享乐。角斗士的力量只为取悦他们而存在。";
        s.choice1 = { "1.暗自发誓一定要打破规则。", [](PlayerStats& p){ p.martial += 5; p.will += 10; p.hp += 10; } };
        s.choice2 = { "2.莫名的怒火从心中燃起。", [](PlayerStats& p){ p.hp -= 10; p.will -= 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "高傲的贵族：瞧啊，一个看起来普普通通的乡下女孩，企图攀上权力和力量的顶峰。";
        s.choice1 = { "1.亮出手中的剑。", [](PlayerStats& p){ p.martial += 5; p.will += 10; } };
        s.choice2 = { "2.不试试怎么知道。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "高傲的贵族：你能为我带来什么？是在场上当滑稽的马戏团演员，被对手追着打，惹得观众大笑？还是觉得靠撒娇就能获得同情？";
        s.choice1 = { "1.别小瞧我。", [](PlayerStats& p){ p.martial += 5; p.will += 10; } };
        s.choice2 = { "2.我从不让人失望。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "高傲的贵族：在座的各位，有愿意选择她的吗？其实看着弱者挣扎，也未尝不是一种乐趣。";
        s.choice1 = { "1.心中闪过一丝期待，希望有人能选中自己。", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.冷漠地瞧着贵族们交头接耳。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "老派的贵族：乐趣……嘿嘿，罗马向来以力量论高低，我们可不应该对这位自荐的角斗士带有这么多偏见啊。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你循声望去，一位衣着讲究的老派贵族正迈着沉重的步伐，从观众席上缓缓走下，最后停在了你的身前。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你在他身上闻到了腐臭的味道，那不是罗马人口中乡下来的老鼠的恶臭，而是权力与金钱的腐味。";
        s.choice1 = { "1.紧紧地握着拳头。", {} };
        s.choice2 = { "2.皱眉看向老派的贵族。", {} };
        act2.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "老派的贵族：我从不做亏本的买卖，你想站在竞技场上就得向我证明你有资格。先通过我安排的三道试炼吧，勇敢的小姐。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act2.scenes.push_back(s);
    }

    acts.push_back(move(act2));

    //Act 3: 第三幕（威望门槛≥240）
    Act act3;
    act3.name = "烈日昔叙之诗 · 第三幕 不可能的试炼";
    act3.prestigeThreshold = 240;

    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你接受了贵族的试炼。与其说是试炼，不如说是他们为刁难你出的难题。";
        s.choice1 = { "1.前往试炼之地。", {} };
        s.choice2 = { "2.前往试炼之地。", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：在罗马城外，有一片被战火侵蚀的废土。那里，正是你的故乡。";
        s.choice1 = { "1.回到故乡，有些伤感。", [](PlayerStats& p){ p.will -= 10; } };
        s.choice2 = { "2.与记忆里的故乡完全不同。", [](PlayerStats& p){ p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：据说，有一头暴戾狮兽常在那片破碎的土地上出没。狮兽白日里藏于洞穴深处，普通的剑根本无法刺穿它粗砺的皮肤。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你的第一道试炼便是将它斩杀，而贵族夺走了你所有的防身装备，只留给你一把钝剑。";
        s.choice1 = { "1.钝剑也足够了。", [](PlayerStats& p){ p.martial += 10; p.will += 10; } };
        s.choice2 = { "2.没办法，只能接受。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：城外的山道蜿蜒如蛇，焦黑的枯树下躺着十几具骸骨。你猜测应该快到目的地了，忍受着难闻的气味继续前行。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Other;
        s.content = "？？？：嘘……蹲下，你不要命了！屏住呼吸，别让它注意到你！";
        s.choice1 = { "1.挣扎。", [](PlayerStats& p){ p.martial += 5; p.prestige -= 10; } };
        s.choice2 = { "2.照着他说的做", [](PlayerStats& p){ p.hp += 10; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：待狮兽离开后，你向猎人道谢。多亏他，你才没被那头怪物发现。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Hunter;
        s.content = "猎人：看你的打扮，是罗马的角斗士？啊，我也曾是你们的一员……我们啊，都不过是贵族的消遣罢了。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：猎人一身的本领现在只能用来挣扎着求生……你自觉这是多么的可悲。";
        s.choice1 = { "1.告诉猎人自己要做什么。", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.向猎人询问狮兽的信息。", [](PlayerStats& p){ p.martial += 5; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：几日后，你已经掌握了狮兽大部分的习性。最终，你向那看不见底的洞穴走去。";
        s.choice1 = { "1.进入洞穴。", {} };
        s.choice2 = { "2.进入洞穴。", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你在幽暗的洞窟中心翼翼地前行……狮兽的金鬃间跃动着冥火，利爪在地面犁出沟壑。";
        s.choice1 = { "1.和狮兽保持一定的距离，挥剑。", [](PlayerStats& p){ p.hp += 10; p.will += 5; } };
        s.choice2 = { "2.拔剑，刺向狮兽。", [](PlayerStats& p){ p.will += 10; } };
        s.entersBattle = true; //第二场战斗：狮兽
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你用尽所有气力朝着洞窟室顶端的钟乳石砸去……那怪物竟被自己活活勒死。";
        s.choice1 = { "1.终于......", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.借助外力，也是一种手段。", [](PlayerStats& p){ p.martial += 5; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你将沾满污垢的双手深入水中……你第一次意识到——力量，足以让人畏惧。";
        s.choice1 = { "1.有些惊魂未定。", [](PlayerStats& p){ p.hp += 10; p.prestige -= 10; } };
        s.choice2 = { "2.渴望更多的力量。", [](PlayerStats& p){ p.martial += 10; p.will += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "老派的贵族：第二道试炼……为我夺来一百条\"命\"吧。";
        s.choice1 = { "1.不屑地转身，做出来接受的手势。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        s.choice2 = { "2.冷静地接受。", [](PlayerStats& p){ p.will += 5; p.prestige += 5; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "围观的罗马人：瞧啊，又是那个女孩……她能一直赢下来？";
        s.choice1 = { "1.关注眼前的对手。", [](PlayerStats& p){ p.will += 5; p.prestige += 5; } };
        s.choice2 = { "2.不在意他人的评价。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白:你习惯了这样的嘈杂……你的剑，此时已经停在了对手的脖颈上。";
        s.choice1 = { "1.放过手高开，等待下一位挑战者。", [](PlayerStats& p){ p.prestige += 10; } };
        s.choice2 = { "2.收住杀意。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "贵族的仆从：你知道维克多少爷的父亲是谁吗？那位贵族老爷，可是大角斗赛最大的赞助人。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "贵族的仆从：权力、财富，这些是蛮力撼动不了的东西。维克多少爷拥有最好的训练导师，握有最精良的武器……";
        s.choice1 = { "1.嗯？所以呢？", [](PlayerStats& p){ p.prestige += 5; } };
        s.choice2 = { "2.的确如此。", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：不久之后，维克多家族找上了你。他们向你递出了一份契约——一份完美的工作和一笔不菲的财富——让你取消角斗。";
        s.choice1 = { "1.接受贿赂。", {}, 1, true, "旁白：你加入了贵族，从此金钱成为了你唯一拥有的东西，你忘了自己一开始所要追求的到底是什么，后悔一直萦绕在脑海，使意志逐渐消减，也消失在了人们的视野中，贵族的腐败将持续到什么时候......" };
        s.choice2 = { "2.撕毁契约。", [](PlayerStats& p){ p.will += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你毅然拒绝了贿赂，贵族的仆从恼羞成怒地咒骂着你。";
        s.choice1 = { "1.保持沉默，不做口舌之争。", [](PlayerStats& p){ p.will += 5; p.prestige -= 10; } };
        s.choice2 = { "2.冷静回应。", [](PlayerStats& p){ p.will += 5; p.prestige += 5; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "贵族的仆从：不懂规矩的乡下人，你知道拒绝贵族意味着什么吗？嗯，就抱着你的自大受折磨去吧，别指望有人可怜你。";
        s.choice1 = { "1.审视眼前的少爷。", [](PlayerStats& p){ p.will += 5; p.prestige += 5; } };
        s.choice2 = { "2.无视仆从。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：贵族少爷拦住了喋喋不休的仆从，起身撕毁了契约。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "维克多：走吧，和无知者交谈简直是浪费时间，当然，也希望你在竞技场上不要浪费我太多时间。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "维克多：他们丢下一袋钱币羞辱着你，讥讽的言语、嘲笑的动作，他们试图利用这些在气势上压倒你。";
        s.choice1 = { "1.原话奉还。", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.不过如此。", [](PlayerStats& p){ p.will += 10; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：到了约定的角斗日，维克多背负着贵族荣誉接受了你的挑战。你想起前几日他那嘲讽的姿态。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：然而贵族嚣张的气焰全是嘴上功夫，对于剑术，维克多并不是很擅长。你轻松击败了这位懦弱无能的贵族少爷，顺便一剑挑起了他滚落在地的漂亮头盔。";
        s.choice1 = { "1.羞辱看台上的贵族。", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.走向贵族的观众席。", [](PlayerStats& p){ p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Solervia;
        s.content = "索尔维娅：我做到了，我没有辜负你们的时间。看见了吧？你们用金钱和权势装点的骄傲，在力量面前不堪一击！";
        s.choice1 = { "1.将头盔丢给维克多。", [](PlayerStats& p){ p.prestige += 10; } };
        s.choice2 = { "2.将维克多的头盔踢到脚下。", [](PlayerStats& p){ p.will += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：在场的观众纷纷为你喝彩，就连那些虚伪的贵族也不得不向你低头致意。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你回到住所清点起战利品，九十九把剑，九十九个倒下的对手。现在，只差最后一条\"命\"……大多角斗士早已畏惧你的名号不敢登场迎战，除了那一位……";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：在罗马，值得一战的对手终究有限……你找上了地下竞技场活跃的新星——亚里德尔。";
        s.choice1 = { "1.久经磨砺的剑未尝不锋利。", [](PlayerStats& p){ p.martial += 5; p.will += 5; } };
        s.choice2 = { "2.天赋与资质我自己不在乎了。", [](PlayerStats& p){ p.will += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：在人们高呼\"亚里德尔\"的喊声中，你如约到场。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：";
        s.choice1 = { "1.老派的贵族这次竟然也在场。", {} };
        s.choice2 = { "2.一如往常，场地周围挤满了看客。", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：对手的体格远胜于你，他所带来的压迫感，让你想起了那头骇人的狮兽。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：几次交锋后，你察觉到了一丝异常。脚下的地板太滑，你几乎无法发力，而亚里德尔的盔甲则重得离谱……贵族的卑劣手段，你再熟悉不过了。";
        s.choice1 = { "1.心生怒火。", [](PlayerStats& p){ p.hp -= 10; p.will -= 10; p.prestige -= 10; } };
        s.choice2 = { "2.保持冷静。", [](PlayerStats& p){ p.hp += 10; p.will += 10; p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你顺势调整节奏，引导亚里德尔落入那本是为你设下的陷阱。几轮过后，他的脚步逐渐变得松散，反应也愈发迟钝。";
        s.choice1 = { "1.就是现在。", [](PlayerStats& p){ p.martial += 10; } };
        s.choice2 = { "2.抓住机会。", [](PlayerStats& p){ p.martial += 10; } };
        s.entersBattle = true;//与亚里德尔对决
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你的剑横在了罗马至强的角斗士面前，全场顿时一片寂静。但片刻后，人群骤然沸腾。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "观众：刺下去，刺下去，刺下去……";
        s.choice1 = { "1.是嘛……", {} };
        s.choice2 = { "2.什么......", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "观众：杀了他，杀了他，杀了他……";
        s.choice1 = { "1.现在？", {} };
        s.choice2 = { "2.这里？", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Aridel;
        s.content = "亚里德尔：……（闭上眼睛等待死亡）……";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：";
        s.choice1 = { "1.胜局已定。", [](PlayerStats& p){ p.prestige += 10; }, 1 };
        s.choice2 = { "2.那就到此为止吧......", {}, 2 };
        act3.scenes.push_back(s);
    }
    //分支1：只取走亚里德尔的剑
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：在众人的目光中，你只取走了亚里德尔手中的剑。随后你向众人行了一礼，转身离去。";
        s.choice1 = { "1.如释重负。", [](PlayerStats& p){ p.will += 5; p.prestige += 5; }, 2 };
        s.choice2 = { "2.感受着人们的注视。", [](PlayerStats& p){ p.prestige += 10; }, 2 };
        act3.scenes.push_back(s);
    }
    //分支2：斩下首级
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：在众人的目光中，亚里德尔的首级应声落地，贵族们糜烂的骄傲与狂妄在此刻溃败，观众席一片哗然，你没有停留，转身离去。";
        s.choice1 = { "1.如释重负。", [](PlayerStats& p){ p.will += 5; p.prestige += 5; } };
        s.choice2 = { "2.感受着人们的注视。", [](PlayerStats& p){ p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "观众：……";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "观众：活着！活着！活着！";
        s.choice1 = { "1.将呐喊声甩在身后。", [](PlayerStats& p){ p.will += 5; p.prestige -= 5; } };
        s.choice2 = { "2.回望向人群，挥手致意。", [](PlayerStats& p){ p.prestige += 10; } };
        act3.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Noble;
        s.content = "老派的贵族：哼，那亚里德尔也不过是罗马人的顶峰，在这百人之上还有更强大的力量。";
        s.choice1 = { "1.超越罗马人的力量。", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.来吧，第三道试炼。", [](PlayerStats& p){ p.will += 10; } };
        act3.scenes.push_back(s);
    }

    acts.push_back(move(act3));

    //Act 4: 第四幕（无名望门槛）
    Act act4;
    act4.name = "烈日昔叙之诗 · 第四幕 立于罗马之巅";
    act4.prestigeThreshold = -1;

    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "试炼前夕，你再次回到训练场。在等待卡托的间隙，你不自觉地来到熟悉的训练木桩前。它是你在追寻力量的道路上，陪伴你最久的朋友。";
        s.choice1 = { "1.斩断它，与过去告别。", [](PlayerStats& p){ p.martial += 10; p.prestige += 5; } };
        s.choice2 = { "2.轻轻拥抱它。", [](PlayerStats& p){ p.martial += 5; p.will += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：怎么样？这条路不好走吧。告诉我，你在这条路上都看见了什么？";
        s.choice1 = { "1.力量，极具破坏性。", [](PlayerStats& p){ p.martial += 5; p.will += 5; p.prestige += 10; } };
        s.choice2 = { "2.力量，足以让人畏惧。", [](PlayerStats& p){ p.martial += 10; p.will += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Cato;
        s.content = "卡托：这代表是征服，是人类最古老律条的延续。索尔维娅，我会去见证你最后的试炼，我会高呼你的名字。";
        s.choice1 = { "1.谢谢。", [](PlayerStats& p){ p.prestige += 5; } };
        s.choice2 = { "2.我不会就此止步。", [](PlayerStats& p){ p.will += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你回到住所，望着房间里堆叠的佩剑出神。这些战利品是力量、是征服的象征，可这份力量与征服，又该如何让世人知晓？";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你曾听闻一位传奇角斗士的故事。那位角斗士总会在胜利后，将对手的剑与自己的武器熔为一体。所以如今，你也带着剑，走到铁匠铺门前。";
        s.choice1 = { "1.凝视铁匠铺中飞溅的火花。", {} };
        s.choice2 = { "2.停在铁匠铺门外。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "展示力量却依然要借助外力……你望向铁匠铺中起落不息的锻锤，心中忽然冒出一个不可思议的念头。";
        s.choice1 = { "1.（继续）", {} };
        s.choice2 = { "2.（继续）", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "——于是，你自己亲手，将所有的剑熔铸在了一起。";
        s.choice1 = { "1.感受着这股力量。", [](PlayerStats& p){ p.martial += 10; } };
        s.choice2 = { "2.握着手中的重剑。", [](PlayerStats& p){ p.martial += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：时间终于来到了试炼日。你带着\"新铸\"的剑走到竞技场中央，青铜号角在十二根石柱的顶端齐鸣。所有人都想见证人类的力量能否撼动狮鹫诺索。";
        s.choice1 = { "1.深吸一口气。", [](PlayerStats& p){ p.will += 10; } };
        s.choice2 = { "2.环视四周。", [](PlayerStats& p){ p.prestige += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：忽地，看台上的呼喊声被某种邃古的威势截断，飘动的贵族旌旗霎地垂落。巨大的阴影将你周身的阳光完全遮蔽——狮鹫索诺现身了。";
        s.choice1 = { "1.有些耳鸣。", [](PlayerStats& p){ p.will -= 10; } };
        s.choice2 = { "2.注意到狮鹫索诺高大如山的身形。", [](PlayerStats& p){ p.martial += 10; p.will += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：\"它醒了！\"某个贵族失声叫道。此时狮鹫诺索正沉默地伫立在你对面，它缓缓睁开双眼，冷冷地审视着你。";
        s.choice1 = { "1.按住剑柄。", [](PlayerStats& p){ p.martial += 5; p.will += 5; } };
        s.choice2 = { "2.提高警惕。", [](PlayerStats& p){ p.martial += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：狮尾颤然扫过石柱，你翻滚着躲开飞溅的碎石，左肩传来布料撕裂的凉意。";
        s.choice1 = { "1.防御。", [](PlayerStats& p){ p.hp += 20; p.will += 10; } };
        s.choice2 = { "2.主动进攻。", [](PlayerStats& p){ p.hp -= 20; p.martial += 10; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：观众席爆发的惊呼被狮吼截断，声浪裹挟着沙粒擦过耳畔，你尝到了唇齿间的血腥味。";
        s.choice1 = { "1.忍耐，伤口以后再说。", [](PlayerStats& p){ p.hp -= 20; p.martial += 10; p.will += 10; } };
        s.choice2 = { "2.迅速处理伤口。", [](PlayerStats& p){ p.hp += 20; } };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：第二波攻击来得毫无预兆，狮鹫前掌拍地的瞬间，你来不及反应，只能用手中的剑正面迎击。";
        s.choice1 = { "1.这一剑，将献出我的所有。", [](PlayerStats& p){ p.martial += 10; } };
        s.choice2 = { "2.回想起之前战斗的历练。", [](PlayerStats& p){ p.martial += 10; } };
        s.entersBattle = true; //最终试炼之战
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你抵挡住了狮鹫诺索的凶猛进攻，在它撤力的一瞬，找准机会将利刃刺入它右前肢的古老裂痕。狮鹫索诺俯下身躯，金色鬃毛轻拂你的面颊。";
        s.choice1 = { "1.我得到了狮鹫的认可。", {} };
        s.choice2 = { "2.我通过了最后的试炼。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Solervia;
        s.content = "索尔维娅：这些声音是......";
        s.choice1 = { "1.聆听沸腾的欢呼声。", {} };
        s.choice2 = { "2.聆听沸腾的欢呼声。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "民众：睿智而庄严、拥有无上伟力的狮鹫诺索啊，请赐予她那用勇气和力量铸成的冠冕吧。";
        s.choice1 = { "1.在狮鹫诺索的引领下登上阶梯。", {} };
        s.choice2 = { "2.在狮鹫诺索的引领下登上阶梯。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "民众：当胜者的呐喊与地脉共鸣，我们便看见人间的火种正在罗马燃烧。";
        s.choice1 = { "1.喝彩如潮，罗马人正向我的前行致敬。", {} };
        s.choice2 = { "2.喝彩如潮，罗马人正向我的前行致敬。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::RomanCrowd;
        s.content = "民众：见证吧，太阳与月亮将见证这非凡的壮举，见证她为罗马献身的史诗。";
        s.choice1 = { "1.再向前一步。", {} };
        s.choice2 = { "2.再向前一步。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：高台升起，你这才明白在获得狮鹫诺索认可后，罗马人将为你开设特例。";
        s.choice1 = { "1.众人在向我行礼。", {} };
        s.choice2 = { "2.众人在向我行礼。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Narrator;
        s.content = "旁白：你将越过大角斗赛，直接加冕为【冠军】。尽管在这之前，你已经用力量征服了无数角斗士。万众瞩目，此刻，荣耀倾服在你脚下。";
        s.choice1 = { "1.高举自己的剑。", {} };
        s.choice2 = { "2.让所有人安静下来。", {} };
        act4.scenes.push_back(s);
    }
    {
        Scene s; s.speaker = Speaker::Neso;
        s.content = "涅索：你荣登名为【冠军】的巅峰，直至放松下来，才真正感觉到伤口带来的痛楚。你以力量打破了腐朽的秩序，那么，你也将以力量重塑新的规则。";
        s.choice1 = { "1.（结束）", {} };
        s.choice2 = { "2.（结束）", {} };
        act4.scenes.push_back(s);
    }

    acts.push_back(move(act4));
}
//游戏开场横幅
void StoryEngine::printStarBanner() {
    const char* orange = "\x1b[38;5;208m";
    string title = "烈日昔叙之诗";
    int boxWidth = 68;//大致适配常见窗口宽度
    if (boxWidth < 20) boxWidth = 20;
    string border(boxWidth, '*');
    cout << orange << border << Color::Reset << "\n";
    int innerWidth = boxWidth - 2;
    int pad = innerWidth - (int)title.size();
    if (pad < 0) pad = 0;
    int leftPad = pad / 2;
    int rightPad = innerWidth - leftPad - (int)title.size();
    cout << orange << "*" << Color::Reset
         << string(leftPad, ' ')
         << orange << title << Color::Reset
         << string(rightPad, ' ')
         << orange << "*" << Color::Reset << "\n";
    cout << orange << border << Color::Reset << "\n\n";
}
//游戏刚开始时打印前言与“输入1继续”
void StoryEngine::printPrefaceAndWait() {
    const char* yellow = Color::Yellow;
    const char* reset = Color::Reset;
    cout << yellow
         << "你听过“烈日少女”的故事吗？一个不起眼的乡下女孩，一个资质平平的角斗士，一个不受贵族庇护的庶民，竟能在狼群环伺的罗马踏出一条光明的长路。要知道恒星在成为辉煌的烈阳前，不过是宇宙中一片无光的尘埃。"
         << reset << "\n";
    cout << Color::BrightBlack << "输入1继续" << Color::Reset << "\n";
    string in;
    while (true) {
        if (!getline(cin, in)) break;
        if (in == "1") break;
        cout << Color::BrightBlack << "请输入 1 继续：" << Color::Reset;
        cout.flush();
    }
}

