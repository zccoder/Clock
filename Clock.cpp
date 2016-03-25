///////////////////////////////////////////////////////////////
// Clock.cpp文件

#include <stdio.h>
#include <string.h>
#include "string.h"
#include <windows.h>
#include "resource.h"
#include <math.h>

#define PI 3.14159265f
#define UNICODE 1
#define _UNICODE 1

LRESULT __stdcall WndProc(HWND, UINT, WPARAM, LPARAM);
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	char szWindowClass[] = "Clock";

	// 注册窗口类
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, (LPCTSTR)IDI_MAIN);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1); // 此域可以是一个系统定义的颜色值
	//wcex.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH); 
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;

	::RegisterClassEx(&wcex);

	// 创建并显示主窗口
	HWND hWnd = ::CreateWindowEx(NULL, szWindowClass, "时钟",
		WS_CHILD | WS_POPUP | WS_SYSMENU | WS_SIZEBOX,
		//WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,		// X，初始 X 坐标		
		CW_USEDEFAULT, 600, 300, NULL, NULL, hInstance, NULL);
	::ShowWindow(hWnd, nShowCmd);
	::UpdateWindow(hWnd);

	// 进入消息循环
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////
// 消息处理代码

#define IDT_CLOCK 1
const int IDM_HELP = 100;
const int IDM_TOPMOST = 101;
COLORREF crfColor;

// 实现函数
//设置坐标系
void SetIsotropic(HDC hdc, int cx, int cy, int x_zuobiao, int y_zuobiao);
//绘制表盘
void DrawClockFace(HDC hdc);
//绘制时针和分针
void DrawHand(HDC hdc, int nLength, int nWidth, int nDegrees, COLORREF clrColor);
//绘制秒针
void DrawHand_add(HDC hdc, int nLength, int nWidth, int nDegrees, COLORREF clrColor);
//写入时间信息
void DrawTime(HDC hdc, int s_nPreHour, int s_nPreMinute, int s_nPreSecond,int space);
//写入日期信息
void DrawDate(HDC hdc, int s_nPreYear, int s_nPreMonth, int s_nPreDay,int space);
//写入周信息
void DrawWeek(HDC hdc,int s_nPreDayOfWeek, int space);
//擦除表盘或文本信息
void CleanClockFace(HDC hdc, int x_zuobiao, int y_zuobiao,int flag);

// 上一次Windows通知时的时间
static int s_nPreHour;		// 小时	
static int s_nPreMinute;	// 分钟
static int s_nPreSecond;	// 秒
//上一次windows通知时的日期
static int s_nPreYear;     //年份
static int s_nPreMonth;    //月份
static int s_nPreDay;      //日期
static int s_nPreDayOfWeek; //周几

static int s_cxClient;
static int s_cyClient;// 窗口客户区的大小
static int s_cClient;
static int cxClient;
static int cyClient;
static int minute_length = 150;//分针长度
static int hour_length = 100;//时针长度
static int minute_width = 2;//分针最宽地方的宽度
static int hour_width = 4;//时针最宽地方的宽度
static int space;//文本信息中设置换行距离
static BOOL s_bTopMost;// 是否位于最顶层
static HWND btnHwnd;//改变时区的按钮

LRESULT __stdcall WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND edit;
	switch (nMsg)
	{
	case WM_CREATE:
	{
					  // ::SetWindowText(hWnd, "计算器");
					  //创建显示文本框
					/*  edit = ::CreateWindow(
						  NULL,
						  "",
						  WS_CHILD | WS_VISIBLE | ES_MULTILINE,
						  60,
						  240,
						  80,
						  20,
						  hWnd,
						  (HMENU)222,
						  ((LPCREATESTRUCT)lParam)->hInstance,
						  NULL);
*/
					  // 向系统菜单中添加自定义的项
					  //	HMENU hSysMenu;
					  // hSysMenu = ::GetSystemMenu(hWnd, FALSE);
					  //::AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
					  // ::AppendMenu(hSysMenu, MF_STRING, IDM_TOPMOST, "总在最前");
					  //::AppendMenu(hSysMenu, MF_STRING, IDM_HELP, "帮助");

					  // 设置时间
					  btnHwnd = CreateWindow(TEXT("button"), TEXT("Change time zone..."),
						  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,        //按钮样式
						  350, 220,        //在父窗口客户区(350, 220)位置创建一个按钮
						  200, 30,    //按钮的大小为200x30
						  hWnd,        //父窗口句柄
						  (HMENU)1,    //按钮控件ID
						  ((LPCREATESTRUCT)lParam)->hInstance,
						  NULL);

					  SYSTEMTIME time;
					  ::GetLocalTime(&time);
					  s_nPreHour = time.wHour % 12;
					  s_nPreMinute = time.wMinute;
					  s_nPreSecond = time.wSecond;
					   
					  s_nPreYear = time.wYear;
					  s_nPreMonth = time.wMonth;
					  s_nPreDay = time.wDay;
					  s_nPreDayOfWeek = time.wDayOfWeek;

					  // 创建定时器
					  ::SetTimer(hWnd, IDT_CLOCK, 1000, NULL);
					
					  return 0;
	}
	case WM_COMMAND:
		   switch (LOWORD(wParam))        //判断子窗口ID, 根据子窗口ID做出不同响应
			     {
			  case 1:        //处理ID为1的子窗口消息
				        switch (HIWORD(wParam))    //通过HIWORD(wParam)进一步判断消息类型
				   {
					case BN_CLICKED:        //处理的按下通知码
						//MessageBox(hWnd, TEXT("按钮被按下!"), TEXT("按钮消息"), MB_OK);
						HDC hdc = ::GetDC(hWnd);
						SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient, s_cyClient);
						CleanClockFace(hdc, 0, 0, 0);

						::SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, NULL,
							SMTO_NORMAL, 3, NULL);
						 break;
					 }
				 break;
				 }
		 return 0;
	case WM_SIZE:
	{
					s_cxClient = LOWORD(lParam);
					s_cyClient = HIWORD(lParam);					
					return 0;
	}
	case WM_SETTINGCHANGE:{
							  DYNAMIC_TIME_ZONE_INFORMATION tziOld, tziNew, tziTest;
							  DWORD dwRet;

							  // Enable the required privilege

							  HANDLE hToken;
							  TOKEN_PRIVILEGES tkp;

							  OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
							  LookupPrivilegeValue(NULL, SE_TIME_ZONE_NAME, &tkp.Privileges[0].Luid);
							  tkp.PrivilegeCount = 1;
							  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
							  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

							  // Retrieve the current time zone information

							  dwRet = GetDynamicTimeZoneInformation(&tziOld);

							  /* if (dwRet == TIME_ZONE_ID_STANDARD || dwRet == TIME_ZONE_ID_UNKNOWN)
							  MessageBox(hWnd, TEXT("dwRet failed (%d)\n"), TEXT("dwRet failed (%d)\n"), MB_OK);
							  else if (dwRet == TIME_ZONE_ID_DAYLIGHT)
							  MessageBox(hWnd, TEXT("dwRet failed (%d)\n"), TEXT("dwRet failed (%d)\n"), MB_OK);
							  else
							  {
							  MessageBox(hWnd, TEXT("dwRet failed (%d)\n"), TEXT("dwRet failed (%d)\n"), MB_OK);
							  return 0;
							  }
							  */
							  // Adjust the time zone information

							  ZeroMemory(&tziNew, sizeof(tziNew));
							  //设定时间差，即bias;
							  int bias = 60;
							  tziNew.Bias = tziOld.Bias + bias;
							  //  StringCchCopy(tziNew.StandardName, 32, L"Test Standard Zone");
							  tziNew.StandardDate.wMonth = 10;
							  tziNew.StandardDate.wDayOfWeek = 0;
							  tziNew.StandardDate.wDay = 5;
							  tziNew.StandardDate.wHour = 2;

							  // StringCchCopy(tziNew.DaylightName, 32, L"Test Daylight Zone");
							  tziNew.DaylightDate.wMonth = 4;
							  tziNew.DaylightDate.wDayOfWeek = 0;
							  tziNew.DaylightDate.wDay = 1;
							  tziNew.DaylightDate.wHour = 2;
							  tziNew.DaylightBias = -60;

							  if (!SetDynamicTimeZoneInformation(&tziNew))
							  {
								  MessageBox(hWnd, TEXT("STZI failed (%d)\n"), TEXT("STZI failed (%d)\n"), MB_OK);
								  return 0;
							  }

							  // Retrieve and display the newly set time zone information

							  dwRet = GetDynamicTimeZoneInformation(&tziTest);
							  /*
							  if (dwRet == TIME_ZONE_ID_STANDARD || dwRet == TIME_ZONE_ID_UNKNOWN)
							  wprintf(L"%s\n", tziTest.StandardName);
							  else if (dwRet == TIME_ZONE_ID_DAYLIGHT)
							  wprintf(L"%s\n", tziTest.DaylightName);
							  else printf("GTZI failed (%d)\n", GetLastError());
							  */
							  // Disable the privilege

							  tkp.Privileges[0].Attributes = 0;
							  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
							  break;
	}

	case WM_PAINT:
	{

					 PAINTSTRUCT ps;
					 HDC hdc = ::BeginPaint(hWnd, &ps);
					
					 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient + 4, s_cyClient + 4);

					 // 经过1个小时时针走30度（360/12），经过1分钟时针走0.5度（30/60）
					 DrawHand_add(hdc, hour_length, hour_width, s_nPreHour * 30 + s_nPreMinute / 2, RGB(112, 128, 144));
					 // 经过1分钟分针走6度（360/60）
					 DrawHand_add(hdc, minute_length, minute_width, s_nPreMinute * 6, RGB(112, 128, 144));

					 // 设置坐标系
					 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient, s_cyClient);

					 DrawClockFace(hdc);				 

					 // 经过1个小时时针走30度（360/12），经过1分钟时针走0.5度（30/60）
					 DrawHand_add(hdc, hour_length, hour_width, s_nPreHour * 30 + s_nPreMinute / 2, RGB(0, 128, 128));
					 // 经过1分钟分针走6度（360/60）
					 DrawHand_add(hdc, minute_length, minute_width, s_nPreMinute * 6, RGB(0, 128, 128));
					 // 经过1秒钟秒针走6度（360/60）
					 DrawHand(hdc, minute_length, 1, s_nPreSecond * 6, RGB(0, 0, 0));
					 // 绘制指针
					 crfColor = ::GetSysColor(COLOR_3DFACE);
					 SetBkColor(hdc, crfColor);

					 // 设置坐标系
					 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient*2, s_cyClient/3);
					 space = s_cyClient / 6;
					 ::TextOut(hdc, 0, 0, "Date:", strlen("Date:"));				 
					// 将日期写入文本框
                     DrawDate(hdc, s_nPreYear, s_nPreMonth, s_nPreDay,-space);
					 //将星期写入文本框
					 DrawWeek(hdc,s_nPreDayOfWeek, -2 * space);
					 ::TextOut(hdc, 0, -4*space, "Time:", strlen("Time:"));
					 //将时间写入文本框
					 DrawTime(hdc,s_nPreHour, s_nPreMinute,  s_nPreSecond,-5*space);
					 ::EndPaint(hWnd, &ps);
					 return 0;
	}

	case WM_TIMER:
	{
					 // 如果窗口处于最小化状态就什么也不做
					 if (::IsIconic(hWnd))	// IsIconic函数用来判断窗口是否处于最小化状态
						 return 0;

					 // 取得系统时间
					 SYSTEMTIME time;
					 ::GetLocalTime(&time);

					 HDC hdc = ::GetDC(hWnd);
					 // 以COLOR_3DFACE为背景色就可以擦除指针了（因为窗口的背景色也是COLOR_3DFACE）				 					
					 // 以COLOR_3DFACE为背景色就可以擦除指针了（因为窗口的背景色也是COLOR_3DFACE）
					 // COLORREF crColor = ::GetSysColor(COLOR_3DFACE);

					 //如果日期改变的话，就擦除日期及周信息并重新写入
					 if (time.wDay != s_nPreDay){
						 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient * 2, s_cyClient / 3);

						 //擦除日期信息
						// CleanClockFace(hdc, 0, -space, 1);
						 //将日期写入文本框
						 DrawDate(hdc, time.wYear, time.wMonth, time.wDay, -space);
						 //擦除周信息
						// CleanClockFace(hdc, 0, -2*space, 1);
						 //将周信息写入文本框
						 DrawWeek(hdc, time.wDayOfWeek, -2 * space);

					 }
					 // 设置坐标系
					 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient + 4, s_cyClient + 4);
					 // 如果分钟改变的话就擦除时针和分针
					 if (time.wMinute != s_nPreMinute)
					 {
						 // 擦除时针和分针
						 //放大针头的宽度达到删除相应的四个指针
						
					//	 DrawHand_add(hdc, hour_length, 40, s_nPreHour * 30 + s_nPreMinute / 2, crfColor);
					//	 DrawHand_add(hdc, minute_length, 40, s_nPreMinute * 6, crfColor);

						 //擦除表盘上的时针和分钟信息
					     CleanClockFace(hdc, 0, 0, 0);
						 s_nPreHour = time.wHour;
						 s_nPreMinute = time.wMinute;
					 }

					 // 如果秒改变的话就擦除秒针，然后重画所有指针
					 //如果秒改变的话就擦除文本，然后重画时间信息
					 if (time.wSecond != s_nPreSecond)
					 {
						 SetBkColor(hdc, crfColor);
			
						 DrawHand_add(hdc, hour_length, hour_width, time.wHour * 30 + time.wMinute / 2, RGB(112, 128, 144));
						 DrawHand_add(hdc, minute_length, minute_width, time.wMinute * 6, RGB(112, 128, 144));

						 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient * 2, s_cyClient / 3);
						 CleanClockFace(hdc, 0, -5 * space, 1);
						 //将时间写入文本框
						 DrawTime(hdc, time.wHour, time.wMinute, time.wSecond, -5 * space);
						 // 擦除秒针
						 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient, s_cyClient);
						 DrawHand(hdc, minute_length, 1, s_nPreSecond * 6, crfColor);

						 // 重画所有指针
						 DrawHand_add(hdc, hour_length, hour_width, time.wHour * 30 + time.wMinute / 2, RGB(0, 128, 128));
						 DrawHand_add(hdc, minute_length, minute_width, time.wMinute * 6, RGB(0, 128, 128));
						 DrawHand(hdc, minute_length, 1, time.wSecond * 6, RGB(0, 0, 0));
						 

						 s_nPreSecond = time.wSecond;
					 }

					 SetIsotropic(hdc, s_cxClient, s_cxClient, s_cxClient + 6, s_cyClient + 6);
					
					 return 0;
	}
	case WM_CONTEXTMENU:
		POINT pt; pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
		{
			// 取得系统菜单的句柄
			HMENU hSysMenu = ::GetSystemMenu(hWnd, FALSE);

			// 弹出系统菜单
			int nID = ::TrackPopupMenu(hSysMenu, TPM_LEFTALIGN | TPM_RETURNCMD,
				pt.x, pt.y, 0, hWnd, NULL);
			if (nID > 0)
				::SendMessage(hWnd, WM_SYSCOMMAND, nID, 0);
			return 0;
		}
	case WM_CLOSE:
	{
					 ::KillTimer(hWnd, IDT_CLOCK);
					 ::DestroyWindow(hWnd);
					 return 0;
	}
	case WM_DESTROY:
	{
					   ::PostQuitMessage(0);
					   return 0;
	}
	case WM_NCHITTEST:
	{
						 UINT nHitTest;
						 nHitTest = ::DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
						 if (nHitTest == HTCLIENT &&
							 ::GetAsyncKeyState(MK_LBUTTON) < 0) // 如果鼠标左键按下，GetAsyncKeyState函数的返回值小于0
							 nHitTest = HTCAPTION;
						 return nHitTest;
	}
	case WM_SYSCOMMAND:
		int nID = wParam;
		{
			if (nID == IDM_HELP)
			{
				::MessageBox(hWnd, "Clock", "Clock", 0);
			}
			else if (nID == IDM_TOPMOST)
			{
				HMENU hSysMenu = ::GetSystemMenu(hWnd, FALSE);
				if (s_bTopMost)
				{
					::CheckMenuItem(hSysMenu, IDM_TOPMOST, MF_UNCHECKED);
					::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);
					s_bTopMost = FALSE;
				}
				else
				{
					::CheckMenuItem(hSysMenu, IDM_TOPMOST, MF_CHECKED);
					::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);
					s_bTopMost = TRUE;
				}
			}
			return ::DefWindowProc(hWnd, WM_SYSCOMMAND, nID, 0);
		}
	}

	return ::DefWindowProc(hWnd, nMsg, wParam, lParam);
}

void SetIsotropic(HDC hdc, int cx, int cy, int x_zuobiao, int y_zuobiao)
{
	::SetMapMode(hdc, MM_ANISOTROPIC);//设置映像模式
	::SetWindowExtEx(hdc, 1000, 1000, NULL);//设置窗口坐标范围
	::SetViewportExtEx(hdc, cx, -cy, NULL);//设置视口坐标范围
	::SetViewportOrgEx(hdc, x_zuobiao / 4, y_zuobiao/2, NULL);
}

// 绘制时钟的外观
void DrawClockFace(HDC hdc)
{
	const int SQUARESIZE = 20;
	const int M_SQUARESIZE = 6;
	double x_result;
	double y_result;
	double ax_result;
	double ay_result;
	POINT pt[4];
	double k = 0;
	double n = 0;
	int base = 180;
	// 画60个圆

	for (int i = 0; i<60; i++)
	{
	    if(i%5==0){
		//::SelectObject(hdc, ::GetStockObject(BLACK_BRUSH));
			int min = base - SQUARESIZE/2;
			int max = base + SQUARESIZE/2;
			x_result = min * cos(n);
			y_result = min * sin(n);
			ax_result = max * cos(n);
			ay_result = max * sin(n);
			double ano_sin = M_SQUARESIZE / 2 * sin(n);
			double ano_cos = M_SQUARESIZE / 2 * cos(n);
			pt[0].x = x_result - ano_sin;
			pt[0].y = y_result + ano_cos;
			pt[1].x = x_result + ano_sin;
			pt[1].y = y_result - ano_cos;
			pt[3].x = ax_result - ano_sin;
			pt[3].y = ay_result + ano_cos;
			pt[2].x = ax_result + ano_sin;
			pt[2].y = ay_result - ano_cos;
			HBRUSH hBrush = ::CreateSolidBrush(RGB(112, 128, 144));
			HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);

			::Polygon(hdc, pt, 4);

			::SelectObject(hdc, hOldBrush);
			::DeleteObject(hBrush);
		}
		else{
			x_result = base * cos(n);
			y_result = base * sin(n);
			::SelectObject(hdc, ::GetStockObject(GRAY_BRUSH));
			::Ellipse(hdc, x_result - M_SQUARESIZE, y_result + M_SQUARESIZE,
				x_result + M_SQUARESIZE, y_result - M_SQUARESIZE);
		}

		k += 6;
		n = (double)k * 0.0174533;
	}

	::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
	::Ellipse(hdc, -base - SQUARESIZE, -base - SQUARESIZE,
	base + SQUARESIZE, base + SQUARESIZE);	
	

}

// 指针的长度、宽度、相对于0点偏移的角度、颜色分别由参数nLength、nWidth、nDegrees、clrColor指定
void DrawHand(HDC hdc, int nLength, int nWidth, int nDegrees, COLORREF clrColor)
{
	// 将角度nDegrees转化成弧度 .	2*3.1415926/360 == 0.0174533
	double nRadians = (double)nDegrees*0.0174533;
	int center = 6;
	// 计算坐标
	POINT pt[2];
	pt[0].x = (int)(nLength*sin(nRadians));
	pt[0].y = (int)(nLength*cos(nRadians));
	pt[1].x = -pt[0].x / 5;
	pt[1].y = -pt[0].y / 5;

	// 创建画笔，并选如DC结构中
	HPEN hPen = ::CreatePen(PS_SOLID, nWidth, clrColor);
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);

	// 画线
	::MoveToEx(hdc, pt[0].x, pt[0].y, NULL);
	::LineTo(hdc, pt[1].x, pt[1].y);

	//画中心圆
	HBRUSH hBrush = ::CreateSolidBrush(RGB(112, 128, 144));
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);
	::Ellipse(hdc, 0 - center, 0 - center,
		0 + center, 0 + center);
	::SelectObject(hdc, hOldBrush);
	::DeleteObject(hBrush);

	::SelectObject(hdc, hOldPen);
	::DeleteObject(hPen);
}

void DrawHand_add(HDC hdc, int nLength, int nWidth, int nDegrees, COLORREF clrColor)
{
	// 将角度nDegrees转化成弧度 .	2*3.1415926/360 == 0.0174533
	double nRadians = (double)nDegrees*0.0174533;
	double nRad = (double)90 * 0.0174533;
	int length = 20;
	// 计算坐标
	POINT pt[4];
	pt[0].x = (int)(nLength*sin(nRadians));
	pt[0].y = (int)(nLength*cos(nRadians));
	pt[2].x = -pt[0].x / 5;
	pt[2].y = -pt[0].y / 5;
	pt[1].x = (int)(length*sin(nRadians - nRad));
	pt[1].y = (int)(length*cos(nRadians - nRad));
	pt[3].x = (int)(length*sin(nRadians + nRad));
	pt[3].y = (int)(length*cos(nRadians + nRad));

	HBRUSH hBrush = ::CreateSolidBrush(clrColor);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);

	// 画线，填充内部
	::Polygon(hdc, pt, 4);
	::SelectObject(hdc, hOldBrush);
	::DeleteObject(hBrush);

	//画边缘，覆盖画线自带的黑色边缘
	HPEN hPen = ::CreatePen(PS_SOLID, nWidth, clrColor);
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);
	::MoveToEx(hdc, pt[0].x, pt[0].y, NULL);
	::LineTo(hdc, pt[1].x, pt[1].y);
	::MoveToEx(hdc, pt[1].x, pt[1].y, NULL);
	::LineTo(hdc, pt[2].x, pt[2].y);
	::MoveToEx(hdc, pt[2].x, pt[2].y, NULL);
	::LineTo(hdc, pt[3].x, pt[3].y);
	::MoveToEx(hdc, pt[3].x, pt[3].y, NULL);
	::LineTo(hdc, pt[0].x, pt[0].y);

	::SelectObject(hdc, hOldPen);
	::DeleteObject(hPen);
}

void DrawTime(HDC hdc, int s_nPreHour, int s_nPreMinute,int s_nPreSecond,int space){

	char hour[20] = "";
	char minute[20] = "";
	char second[20] = "";
	int len;
	_gcvt(s_nPreHour, 15, hour);
	len = strlen(hour);
	if (hour[len - 1] == '.'){
		hour[len - 1] = '\0';
	}

	_gcvt(s_nPreMinute, 15, minute);	//双精度转换为字符串函数
	len = strlen(minute);
	if (minute[len - 1] == '.'){
		minute[len - 1] = '\0';
	}
	_gcvt(s_nPreSecond, 15, second);
	len = strlen(second);
	if (second[len - 1] == '.'){
		second[len - 1] = '\0';
	}

	strcat(hour, ":");
	strcat(hour, minute);
	strcat(hour, ":");
	strcat(hour, second);
	//::setWindowText(edit,hour);
	::TextOut(hdc, 0, space, hour, strlen(hour));
}

void DrawDate(HDC hdc, int s_nPreYear, int s_nPreMonth, int s_nPreDay, int space){

	char text[200] = {};
	char year[20] = "";
	char month[20] = "";
	char day[20] = "";
	char dayOfWeek[20] = "";
	int len;
	_gcvt(s_nPreYear, 15, year);
	len = strlen(year);
	if (year[len - 1] == '.'){
		year[len - 1] = '\0';
	}
	_gcvt(s_nPreMonth, 15, month);
	len = strlen(month);
	if (month[len - 1] == '.'){
		month[len - 1] = '\0';
	}
	_gcvt(s_nPreDay, 15, day);
	len = strlen(day);
	if (day[len - 1] == '.'){
		day[len - 1] = '\0';
	}

	strcat(text, year);
	strcat(text, "年");
	strcat(text, month);
	strcat(text, "月");
	strcat(text, day);
	strcat(text, "日");

	::TextOut(hdc, 0, space, TEXT(text), lstrlen(TEXT(text)));	
}

void DrawWeek(HDC hdc, int s_nPreDayOfWeek, int space){
	char text[200] = {};
	strcat(text, "星");
	strcat(text, "期");
	
	switch (s_nPreDayOfWeek){
	case 1:
		strcat(text, "一");
		break;
	case 2:
		strcat(text, "二");
		break;
	case 3:
		strcat(text, "三");
		break;
	case 4:
		strcat(text, "四");
		break;
	case 5:
		strcat(text, "五");
		break;
	case 6:
		strcat(text, "六");
		break;
	case 0:
		strcat(text, "日");
		break;
	}
	::TextOut(hdc, 0, space, TEXT(text), lstrlen(TEXT(text)));
}

//通过flag为0或1分别对表盘或文本栏进行擦除
//0：擦除表盘
//1：擦除文本
void CleanClockFace(HDC hdc, int x_zuobiao, int y_zuobiao, int flag){
	HBRUSH hBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, hBrush);
	HPEN hPen = ::CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);

	if (flag == 0){
		::Ellipse(hdc, x_zuobiao - minute_length, y_zuobiao - minute_length, x_zuobiao + minute_length, y_zuobiao + minute_length);
		//擦除边缘
		::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
		::Ellipse(hdc, x_zuobiao - minute_length, y_zuobiao - minute_length, x_zuobiao + minute_length, y_zuobiao + minute_length);
	}
	else{
		::Rectangle(hdc, x_zuobiao, y_zuobiao - space, x_zuobiao + s_cxClient, y_zuobiao);
		//擦除边缘
		//画边缘，覆盖画线自带的黑色边缘
		// 计算坐标

		POINT pt[4];
		pt[0].x = x_zuobiao;
		pt[0].y = y_zuobiao;
		pt[1].x = x_zuobiao;
		pt[1].y = y_zuobiao - space;
		pt[2].x = x_zuobiao + s_cxClient;
		pt[2].y = y_zuobiao - space;
		pt[3].x = x_zuobiao + s_cxClient;
		pt[3].y = y_zuobiao;

		::MoveToEx(hdc, pt[0].x, pt[0].y, NULL);
		::LineTo(hdc, pt[1].x, pt[1].y);
		::MoveToEx(hdc, pt[1].x, pt[1].y, NULL);
		::LineTo(hdc, pt[2].x, pt[2].y);
		::MoveToEx(hdc, pt[2].x, pt[2].y, NULL);
		::LineTo(hdc, pt[3].x, pt[3].y);
		::MoveToEx(hdc, pt[3].x, pt[3].y, NULL);
		::LineTo(hdc, pt[0].x, pt[0].y);

		::SelectObject(hdc, hOldPen);
		::DeleteObject(hPen);
	}
}