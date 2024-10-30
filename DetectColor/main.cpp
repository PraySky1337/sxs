#include "rgb.h"
std::string testPath[4] = {"../video/rgb.avi",  //三色测试视频路径
                           "../video/blue.avi", //蓝色测试视频路径
                           "../video/green,avi",//绿色测试视频路径
                           "../video/red.avi"}; //红色测试视频路径
int main()
{
    PlayVideo(30);
    return 0;
}