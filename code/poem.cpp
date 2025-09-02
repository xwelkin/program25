#include "Soler.h"
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;
class StoryEngine {
    public:
        StoryEngine();
        void resetToPrologue();
        void startGameLoop();
    };
int main() { //程序入口
//这些不常见的东西都是请教AI弄来的，轻喷
#ifdef _WIN32 //仅在Windows环境下执行以下设置
    SetConsoleOutputCP(CP_UTF8); //设置控制台输出代码页为UTF-8，保证中文正常显示
    SetConsoleCP(CP_UTF8); //设置控制台输入代码页为UTF-8，保证输入不乱码
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE); //获取标准输出句柄
    DWORD mode = 0; //保存控制台模式标志位
    if (GetConsoleMode(hout, &mode)) { //读取当前控制台模式成功则进入
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; //开启VT序列支持，使ANSI颜色转义生效
        SetConsoleMode(hout, mode); //回写更新后的控制台模式
    }
#endif //结束Windows平台相关设置
StoryEngine engine; //构造剧情引擎
engine.resetToPrologue(); //重置到序幕并初始化角色属性
engine.startGameLoop(); //开始运行游戏主流程
return 0; //正常结束程序
}





