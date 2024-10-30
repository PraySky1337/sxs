#include "rgb.h"
#include <string>
using namespace std;
using namespace cv;
int lowerH[3], lowerS[3], lowerV[3];
int upperH[3], upperS[3], upperV[3];
Scalar lower_value[3]; // BGR三色最低阈值
Scalar upper_value[3]; // BGR三色最高阈值
int erodeNum[3];       // BGR三色图像腐蚀次数
int dilationNum[3];    // BGR三色膨胀次数



static void ProcessImg(Mat &frame, int color_choice, vector<vector<Point>> &contours);
static void DrawRect(const vector<vector<Point>> &contours, Mat &frame);
static void FileRead();
static void CreateTrackBar();
static void callback(int value, void *);
static void Save();




//主函数
int PlayVideo(int fps, string path)
{
    // 初始化全部参数
    FileRead();
    CreateTrackBar();
    Mat frame, mask;
    VideoCapture cap;
    if(path == "defaultCamera")
    cap.open(0);
    else cap.open(path);
        
    
    if (!cap.isOpened())
    {
        cerr << "请检查视频是否存在！";

        return -1;
    }

    while (waitKey(1000/fps) != 27)
    {
        cap >> frame;
        if (frame.empty())
        {
            break;
        }
        // 处理图像，返回三组点的集合
        vector<vector<Point>> contours;
        ProcessImg(frame, 0, contours);
        ProcessImg(frame, 1, contours);
        ProcessImg(frame, 2, contours);
        // 绘制轮廓并简化轮廓
        if (!contours.empty())
        {
            DrawRect(contours, frame);
        }
        imshow("video", frame);
    }
    Save();
    return 0;
}




void ProcessImg(Mat &frame, int color_choice, vector<vector<Point>> &contours)
{
    Mat mask, hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);
    // 高斯滤波
    GaussianBlur(hsv,hsv,Size(5,5),0);
    // 二值化
    if(color_choice == 2){ //HSV中的红色H范围有两个，mask掩码合并一下。
        Mat mask1,mask2;
        inRange(hsv,Scalar(0,lowerS[2],lowerV[2]),Scalar(7,upperS[2],upperV[2]),mask2);
        inRange(hsv, lower_value[color_choice], upper_value[color_choice], mask1);
        mask = mask1 | mask2;
    }else
    inRange(hsv, lower_value[color_choice], upper_value[color_choice], mask);
    // 腐蚀和膨胀
    for (int i = 0; i < erodeNum[color_choice]; i++)
    {
        erode(mask, mask, getStructuringElement(MORPH_RECT, Size(3, 3)));
    }
    for (int i = 0; i < dilationNum[color_choice]; i++)
    {
        dilate(mask, mask, getStructuringElement(MORPH_RECT, Size(3, 3)));
    }
    //闭运算
    morphologyEx(mask,mask,MORPH_CLOSE,getStructuringElement(0,Size(5,5)));
    vector<vector<Point>> temp_contours;
    findContours(mask, temp_contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    if (color_choice == 0)
        imshow("maskblue", mask);
    if (color_choice == 1)
        imshow("maskgreen", mask);
    if (color_choice == 2)
        imshow("maskred", mask);
    // 计算轮廓中面积的最大值
    // 如果没有，那就不返回
    vector<Point> largest_contour; // 最大面积的点集
    double max_area = 0;           // 面积最大值

    // 遍历面积，找到最大面积
    if (!temp_contours.empty())
    {
        for (const auto &it : temp_contours)
        {
            if (contourArea(it) > max_area)
            {
                max_area = contourArea(it);
                largest_contour = it;
            }
        }
        if(!largest_contour.empty())
        contours.push_back(largest_contour);
    }
}




// 函数的定义
void DrawRect(const vector<vector<Point>> &contours, Mat &frame)
{
    for (size_t i = 0; i < contours.size(); i++)
    {
        if (contours[i].empty()) continue;
        RotatedRect minrect = minAreaRect(contours[i]);
        Point2f points[4];
        minrect.points(points);
        Point2f cpt = minrect.center;
        for (int i = 0; i < 4; i++)
        {
            line(frame, points[i], points[(i + 1)%4], Scalar(0, 255, 0), 2, 8, 0);
        }
        circle(frame, cpt, 2, Scalar(0, 255, 0), 2, 8, 0);
    }
}

string path = "data.yaml";






void FileRead()
{
    FileStorage fread(path, FileStorage::READ);
    if (fread.isOpened())
    {
        string node_name[8] = {"lowerH", "lowerS", "lowerV", "upperH", "upperS", "upperV", "erodeNum", "dilationNum"};
        int *value_name[8] = {lowerH, lowerS, lowerV, upperH, upperS, upperV, erodeNum, dilationNum};
        FileNode file_node[8];
        for (int i = 0; i < 8; i++)
        {
            file_node[i] = fread[node_name[i]];
        }
        for (int i = 0; i < 8; i++)
        {
            int temp = 0; // 数值等同于迭代器迭代次数,作为Valuename的二维下标
            for (auto it = file_node[i].begin(); it != file_node[i].end(); it++)
            {
                *it >> value_name[i][temp++];
            }
        }
    }
    fread.release();
    // 初始化scalar对象
    for (int i = 0; i < 3; i++)
    {
        lower_value[i] = Scalar(lowerH[i], lowerS[i], lowerV[i]);
        upper_value[i] = Scalar(upperH[i], upperS[i], upperV[i]);
    }
}





void Save()
{
    FileStorage fwrite(path, FileStorage::WRITE);
    if (fwrite.isOpened())
    {
        fwrite << "lowerH" << "[" << lowerH[0] << lowerH[1] << lowerH[2] << "]";
        fwrite << "lowerS" << "[" << lowerS[0] << lowerS[1] << lowerS[2] << "]";
        fwrite << "lowerV" << "[" << lowerV[0] << lowerV[1] << lowerV[2] << "]";
        fwrite << "upperH" << "[" << upperH[0] << upperH[1] << upperH[2] << "]";
        fwrite << "upperS" << "[" << upperS[0] << upperS[1] << upperS[2] << "]";
        fwrite << "upperV" << "[" << upperV[0] << upperV[1] << upperV[2] << "]";
        fwrite << "erodeNum" << "[" << erodeNum[0] << erodeNum[1] << erodeNum[2] << "]";
        fwrite << "dilationNum" << "[" << dilationNum[0] << dilationNum[1] << dilationNum[2] << "]";
    }
    fwrite.release();
}




void callback(int value, void *)
{
    for (int i = 0; i < 3; i++)
    {
        lower_value[i] = Scalar(lowerH[i], lowerS[i], lowerV[i]);
        upper_value[i] = Scalar(upperH[i], upperS[i], upperV[i]);
    }
}





void CreateTrackBar()
{
    string window_name[3] = {"BlueTrackBar", "GreenTrackBar", "RedTrackBar"};
    int *value_name[8] = {lowerH, lowerS, lowerV, upperH, upperS, upperV, erodeNum, dilationNum};
    namedWindow(window_name[0],WINDOW_NORMAL);
    namedWindow(window_name[1],WINDOW_NORMAL);
    namedWindow(window_name[2],WINDOW_NORMAL);

    string node_name[8] = {"lowerH", "lowerS", "lowerV", "upperH", "upperS", "upperV", "erodeNum", "dilationNum"};
    for (int i = 0; i < 8; i++)
    {
        int max_value; // 滑动杆最大值
        if (i == 0 || i == 3)
        {
            max_value = 180;
        }
        else if (i == 6 || i == 7)
        {
            max_value = 8;
        }
        else
        {
            max_value = 255;
        }
        createTrackbar(node_name[i], window_name[0], &value_name[i][0], max_value, callback, 0);
        createTrackbar(node_name[i], window_name[1], &value_name[i][1], max_value, callback, 0);
        createTrackbar(node_name[i], window_name[2], &value_name[i][2], max_value, callback, 0);
    }
}