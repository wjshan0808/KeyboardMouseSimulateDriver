//mouse 鼠标控制
import winex;
namespace winex.mouse; 

//定义延时必数、时间间隔
delay = ..win.delay
delayDown  = 5;
delayUp = 1;
delayClick = 5;

//延时函数,用局部函数使下面所有名字空间可以访问
var downDelay = function(){
	delay( delayDown )
}
var upDelay = function(){
	delay( delayUp )	
}
var clickDelay = function(){
	if( delayClick > delayUp )
		delay( delayClick - delayUp )	
}

var send = function(hwnd,cmd,wParam,x=0,y=0){ 
    var lParam = ( ( (y & 0xFFFF) << 16) | (x & 0xFFFF) ) & 0xFFFFFFFF 
	::PostMessage(hwnd,cmd,wParam,lParam);
}

move = function(hwnd,x=0,y=0){
	send(hwnd,0x200,1,x,y)
}

down = function(hwnd,x=0,y=0){
	send(hwnd,0x201,1,x,y)
	downDelay()
}
up = function(hwnd,x=0,y=0){
	send(hwnd,0x202,1,x,y)
	upDelay();
}
click = function(hwnd,x=0,y=0){
	down(hwnd,x,y); 
	up(hwnd,x,y);
	clickDelay();
}
clickDB = function(hwnd,x,y){
	send(hwnd,0x203,1,x,y)
	clickDelay();
}

namespace mb{
	roll = function(hwnd,x,y,delta){
		send(hwnd,0x20A/*_WM_MOUSEWHEEL*/,( (delta & 0xFFFF) << 16) | 0x10,x,y) ;
	}
	down = function(hwnd,x,y){
		send(hwnd,0x207,0x10,x,y)
		downDelay()
	}
	up = function(hwnd,x,y){
		send(hwnd,0x208,0x10,x,y)
		upDelay();
	}
	click = function(hwnd,x,y){
		down(hwnd,x,y); 
		up(hwnd,x,y);
		clickDelay();
	} 
	clickDB = function(hwnd,x,y){
		send(hwnd,0x209,1,x,y)
		clickDelay();
	}
}

namespace rb{
	down = function(hwnd,x,y){
		send(hwnd,0x204,2,x,y)
		downDelay()
	}
	up = function(hwnd,x,y){
		send(hwnd,0x205,2,x,y)
		upDelay();
	}
	click = function(hwnd,x,y){
		down(hwnd,x,y);  
		up(hwnd,x,y);
		clickDelay();
	} 
	clickDB = function(hwnd,x,y){
		send(hwnd,0x206,1,x,y)
		clickDelay();
	}
}

/*intellisense(winex.mouse)
delay = 指定延时函数，默认为win.delay
delayDown = 按下后的时间间隔,默认为5
delayUp = 弹起后的时间间隔,默认为1
delayClick = 完整击键之间的间隔,默认为5

down(.(句柄,x,y) = 左键按下
up(.(句柄,x,y) = 左键弹起
click(.(句柄,x,y) = 左键单击
clickDB(.(句柄,x,y) = 左键双击
move(.(句柄,x,y) = 移动鼠标
end intellisense*/

/*intellisense(winex.mouse.mb)
down(.(句柄,x,y) = 中键按下 
up(.(句柄,x,y) = 中键弹起
click(.(句柄,x,y) = 中键单击
clickDB(.(句柄,x,y) = 中键双击
roll(.(句柄,x,y,-3) = 向下滚动
roll(.(句柄,x,y,3) = 向上滚动
end intellisense*/

/*intellisense(winex.mouse.rb)
down(.(句柄,x,y) = 右键按下 
up(.(句柄,x,y) = 右键弹起
click(.(句柄,x,y) = 右键单击
clickDB(.(句柄,x,y) = 右键双击
end intellisense*/

    