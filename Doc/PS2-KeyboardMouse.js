import key;
import mouse;
import winring0;
import com;
import win;

namespace winring0; 

namespace ps2;
  
/*
	PS/2 模式: Bit 5 - Mouse 输出缓冲满 
	AT   模式: Bit 5 - TX 超时 
*/
waitMouseOutputEmpty = function(){
   var B;
   do{
	   sleep(1);
     B = ReadPortValue(0x64);
   } while( (B & 0x20) && (B & 0x01) ); //如果标志位是1,则等待
}

waitMouseOutputFull = function(){
   var B;
   do{
	   sleep(0);
     B = ReadPortValue(0x64);
   } while( (!(B & 0x20)) || (!(B & 0x01)) ); //必须这两个标志位都是1
}

waitKeyOutputEmpty = function(){
   var B;
   do{
	    sleep(1);
     B = ReadPortValue(0x64);
   } while(B & 0x01); //如果标志位是1,则等待
}

waitKeyOutputFull = function(){
   var B;
   do{
	   sleep(0);
     B = ReadPortValue(0x64);
   }while( (B & 0x20) || (!(B & 0x01)) ); //如果标志位是0,则等待
}


/*
	在向64h端口写数据之前必须确保Input Register是空的（通过判断Status Register的Bit-1是否为0）。
	无论向0x60,还是0x64写东西前都要等待 状态寄存器 OBF变0( 是相对主机而言，相对键盘而言即是Input Register )
*/
waitInputEmpty = function(){
   var B;
   do{
	    sleep(1);
     B = ReadPortValue(0x64);
   }while(B & 0x02);
}

//下面这个函数在任何时候都不应当用
waitInputFull = function(){
   var B;
   do{
	    sleep(0);
     B = ReadPortValue(0x64);
   }while( !(B & 0x02) );
}

/*
	对60h端口进行读操作，将会读取Output Register的内容。 Output Register的内容可能是：
		来自于8048的数据。这些数据包括Scan Code，对8048发送的命令的确认字节（ACK)及回复数据。
		通过64h端口对8042发布的命令的返回结果。
	在向60h端口读取数据之前必须确保Output Register中有数据（通过判断Status Register的Bit-0是否为1）。
*/
waitOutputFull = function() {
   var B;
   do{
    sleep(0);  
     B = ReadPortValue(0x64);
   }while( !(B & 0x01) ); //这里千万不能用sleep 不然永远不能返回，因为被系统键盘驱动清空了
}

waitOutputEmpty = function(){
   var B;
   do{
	   sleep(1);
     B = ReadPortValue(0x64);
   }while( (B & 0x01) );
} 
 
//异步，不论有没有等到按键都返回值，成功返回true，失败返回false  
waitMouseOutputFullAsyn = function(){
   var B;
   do{
	 sleep(1);  
     B = ReadPortValue(0x64);
   }while( (!(B&0x20)) || (!(B&0x01)) ); //必须这两个标志位都是1
   return true;
}

_WAIT_MAX_ASYN = 20 
waitKeyOutputFullAsyn = function(){
   var i = 0;
   var B;
   do{
	   sleep(1);

	   i++;
	   if(i>=_WAIT_MAX_ASYN) {
		   return false;
	   }

     B = ReadPortValue(0x64);
   }while( (B & 0x20) || (!(B & 0x01)) ); //如果标志位是0,则等待
   return true;
}
 
waitOutputFullAsyn = function() {
   var B;
   do{
     sleep(1); 
     B = ReadPortValue(0x64);
   }while( !(B & 0x01) );

   return true;
}
  
/*
	在向64h端口写某些命令之前必须确保键盘是被禁止的，
	因为这些被写入的命令的返回结果将会放到Output Register中，
	而键盘如果不被禁止，则也会将数据放入到Output Register中，会引起相互之间的数据覆盖；

	打开键盘接口。Command Byte的bit-4被清除。
	当此命令被发布后，Keyboard将被允许发送数据到Output Register。
*/
enableKb = function(  bEnable ){
	if(bEnable) {
		waitInputEmpty();
		WritePortValue(0x64, 0xAE); //启用键盘接口. 清除控制寄存器的bit 4.  
	}
	else {
		waitInputEmpty();
   		WritePortValue(0x64, 0xAD); //禁止键盘接口. 设置控制寄存器的bit 4. 
	}
}
  
/*
0xa7 - n/a - PS/2, VIA 
禁止鼠标接口. 把控制端口bit 5 and P23 置1 

0xa7 - n/a - AMI 
设置内部的 'bad write cache' 标志? 

0xa8 - n/a - PS/2, VIA 
使能鼠标接口. 把控制端口bit 5 and P23 设置为0 

0xa8 - n/a - AMI 
清除内部的 'bad write cache' 标志?
*/
enableMouse = function( bEnable ){
	if(bEnable) {
		waitInputEmpty();
		WritePortValue(0x64, 0xA8); //使能鼠标接口. 把控制端口bit 5 and P23 置0  
	}
	else {
		waitInputEmpty();
   		WritePortValue(0x64, 0xA7); //禁止鼠标接口. 把控制端口bit 5 and P23 置1 . 
	}
}

//60端口写入数据
writeK60 = function( nChr){
	waitInputEmpty(); //等待IBF空闲
	WritePortValue(0x60, nChr); 
}

//64端口写入数据
writeK64 = function( nChr){
	waitInputEmpty(); //等待IBF空闲
	WritePortValue(0x64, nChr); 
}


testMousePS2 = function() {
	waitInputEmpty(); //等待芯版读取指令并清空IBF
	writeK64(0xA9);
	waitInputEmpty(); //等待芯版读取指令并清空IBF
	return (ReadPortValue(0x60) == 0x00);
}

testKeyPS2 = function(){
	waitInputEmpty(); //等待芯版读取指令并清空IBF
	writeK64(0xAB);
	waitInputEmpty(); //等待芯版读取指令并清空IBF
	return (ReadPortValue(0x60) == 0x00);
}

writeScanKey = function(chr){ 
	waitOutputEmpty(); //等待OBF空闲
 
	waitInputEmpty(); //等待IBF空闲
	WritePortValue(0x64, 0xd2);
 
	waitInputEmpty(); //等待芯版读取指令并清空IBF
	WritePortValue(0x60, chr);
 
	waitInputEmpty(); //等待芯版读取数据并清空IBF
 
	waitOutputEmpty(); //等侍系统读取并清空OBF
}
  
var VK_EXT2 = ..key.VK_EXT2;
writeKeyDown = function(vkey){
	if(VK_EXT2[vkey]) {
		writeScanKey(0xE0);
	}

	var scancode = ::MapVirtualKey(vkey, 0);
	writeScanKey(scancode);
}

writeKeyUp = function(vkey){
	if(VK_EXT2[vkey]) {
		writeScanKey(0xE0);
	}

	var scancode = ::MapVirtualKey(vkey, 0);
	writeScanKey(scancode | 0x80);
}
  

var m_mouseMetaData = 0x08; 
/*
鼠标数据第一个字节
	7 y溢出标志
	6 x溢出标志
	5 y信号位
	4 x信号位
	3 这个位总是1,也就是 0x08
	2 鼠标中键
	1 鼠标右键
	0 鼠标左键
第二个字节 x位移 
第三个字节 y位移
*/
writeMouse = function( x = 0, y = 0, nMessage = 0,abs){  
	if( !ps2Mouse ) error("系统未检测到PS2鼠标！",3)
	
	if( abs ){
		..mouse.setPos(x,y)
		x,y = 0, 0;
	}
 
	var dx = (..math.abs(x) & 0xFF);
	var dy = (..math.abs(y) & 0xFF);

	if(y<=0){ //向上移
		m_mouseMetaData  &= ~0x20;
	}
	else{ //向下移 
		 m_mouseMetaData |=  0x20;
		 dy = (~dy + 1);//求补码 取反加1
	}

	if(x<0){ //向左移  
		m_mouseMetaData |=  0x10;
		dx = (~dx + 1);//求补码 取反加1
	}
	else{ //向右移
		m_mouseMetaData  &= ~0x10;
	}

	select(nMessage) {
		case 0x201/*_WM_LBUTTONDOWN*/{ 
		 	m_mouseMetaData |=  0x01;
		}
		case 0x202/*_WM_LBUTTONUP*/{ 
		 	m_mouseMetaData  &= ~0x01;
		}
		case 0x207/*_WM_MBUTTONDOWN*/{ 
		 	m_mouseMetaData |=  0x04;
		}
		case 0x208/*_WM_MBUTTONUP*/{ 
		 	m_mouseMetaData  &= ~0x04;
		}
		case 0x204/*_WM_RBUTTONDOWN*/{ 
		 	m_mouseMetaData |=  0x02;
		}
		case 0x205/*_WM_RBUTTONUP*/{ 
		 	m_mouseMetaData  &= ~0x02;
		}
	} 

	waitMouseOutputEmpty(); //等侍系统读取并清空OBF/

	writeK64( 0xd3 );
	writeK60( m_mouseMetaData );
	waitMouseOutputEmpty(); //等侍系统读取并清空OBF

	writeK64( 0xd3 );
	writeK60( dx );
	waitMouseOutputEmpty(); //等侍系统读取并清空OBF

	writeK64( 0xd3 );
	writeK60( dy );
	waitMouseOutputEmpty(); //等侍系统读取并清空OBF

	if(mouseIs3d){ //滚轮鼠标 
		writeK64( 0xd3 );
		writeK60( 0x00 );   
	}

	waitMouseOutputEmpty(); //等侍系统读取并清空OBF
}

/*
Bit 1 - 鼠标中断使能 
置1后, 控制器会在接受到鼠标的数据后产生IRQ12 (P25)中断. 

Bit 0 - 键盘中断使能 
置1后, 控制器会在接受到键盘数据后产生IRQ1 (P24) 中断.

在对0x60端口读后，会改变0x64端口对应控制寄存器的状态
*/
enableKbIrq = function(bEnable) {
	var state;
	
	enableKb(false); //
	enableMouse(false); //鼠标数据是永远不会空的，只有禁用他 

	writeK64( 0x20 );
	waitInputEmpty();
	//waitOutputFull(); //读指令以前不能要这句，要了的话的，等读的时候已经被系统驱动程序读走了
	state = ReadPortValue(0x60);
	if(bEnable) {
		state |=  0x01;
	}
	else {
		state &= ~0x01;
	}

	writeK64( 0x60 );
	writeK60( state);

	enableKb(true);
	enableMouse(true); 
} 
 
//按下返回true 弹起返回false
reakKey = function( sendkey = true ) { 
	var k;
	//waitOutputEmpty();  //不可以在设置中断的代码内部任何位置调用这句
	{
		enableKbIrq(false);
		if(  !( waitKeyOutputFullAsyn() )  ) {
			enableKbIrq(true);
			return 0;
		}

		k =ReadPortValue(0x60);
		if(k==0xe0)  {   
			//下面这个键不能用异步的，因为一定有两个指令
			waitKeyOutputFull(); //等待键盘有按键发送过来 OBF置1
			k =ReadPortValue(0x60);
			//从i8042读出扩展码的0xe0或者0xe1时，会设置这里，随后读入的数据，就知道是扩展码的一部分。
			//两字节扫描码，等待读取下一个字节，特殊键
		}
		
		enableKbIrq(true); 
	}


	if(k==0xe1){ /*扫描码为E1开头为Pause键*/ 
		return 0;
	}
	else {
		if(k & 0x80)  //判断从i8042读入的数据是否大于 0x7F，也就是判断是 Make Code 还是 Break Code。
		{
			k = ::MapVirtualKey((k &~ 0x80),1);

			if(sendkey) {
				writeKeyUp(k);
				//sleep(1);
			}
			return k,false;//弹起
		}
		else {
			k = MapVirtualKey(k,1);

			if(sendkey) {
				WriteKeyDown(k);
				//sleep(1);
			}
			return k,true;//按下
		}
	}
 
	return 0;
}

readMouseID = function() { 
	waitMouseOutputEmpty();
    writeK64( 0xD4);
    writeK60( 0xF2);
    return ReadPortValue(0x60);  
}


//前面是硬件指令级函数，以下是用户接口函数 

delay = ..win? ..win.delay: sleep;
delayPress = 5;
delayDown = 5;
delayUp = 1;

keyDown = function(key){
	var key = ..key.getCode(key)
	writeKeyDown(key);
	delay(delayDown)
}

keyUp = function(key){
	var key = ..key.getCode(key)
	writeKeyUp(key);
	delay(delayUp)
}

keyPress = function(key){
	var key = ..key.getCode(key)
	writeKeyDown(key);
	delay(delayDown);
	writeKeyUp(key);
	delay(delayUp)	
}

combine = function(...){
	var vk,ext;
	var args ={ ... }; 
	
    for(i=1;#args;1){
    	keyDown(args[i]);
    }
    
    delay( delayDown );
    
    for(i=1;#args;1){
       keyUp(args[i]);
    }
    delay( delayPress );
}

mouseMove = function(x,y,abs){ 
	writeMouse(x,y,0,abs); 
}

mouseDown = function(x,y,abs){ 
	writeMouse(x,y,0x201/*_WM_LBUTTONDOWN*/,abs);
	delay(delayDown)
}

mouseUp = function(x,y,abs){
	writeMouse(x,y,0x202/*_WM_LBUTTONUP*/,abs);
	delay(delayUp)
}

mousePress = function(x,y,abs){
	writeMouse(x,y,0x201/*_WM_LBUTTONDOWN*/,abs);
	delay(delayDown)
	writeMouse(x,y,0x202/*_WM_LBUTTONUP*/,abs);	
	delay(delayUp)
}

mouseDownRb = function(x,y,abs){
	writeMouse(x,y,0x204/*_WM_RBUTTONDOWN*/,abs);
	delay(delayDown)
}

mouseUpRb = function(x,y,abs){
	writeMouse(x,y,0x205/*_WM_RBUTTONUP*/,abs);
	delay(delayUp)
}

mousePressRb = function(x,y,abs){
	writeMouse(x,y,0x204/*_WM_RBUTTONDOWN*/,abs);
	delay(delayDown)
	writeMouse(x,y,0x205/*_WM_RBUTTONUP*/,abs);	
	delay(delayUp)
}

mouseDownMb = function(x,y,abs){
	writeMouse(x,y,0x207/*_WM_MBUTTONDOWN*/,abs);
	delay(delayDown)
}

mouseUpMb = function(x,y,abs){
	writeMouse(x,y,0x208/*_WM_MBUTTONUP*/,abs);
	delay(delayUp)
}

mousePressMb = function(x,y,abs){
	writeMouse(x,y,0x207/*_WM_MBUTTONDOWN*/,abs);
	delay(delayDown)
	writeMouse(x,y,0x205/*_WM_RBUTTONUP*/,abs);
	delay(delayUp)	
}

getPs2Device = function(){
   var ps2Mouse,ps2Keyboard = null; 
   
   var locator = ..com.CreateObject("WbemScripting.SWbemLocator")  
   var service = locator.ConnectServer("."); 
   var mouses = service.ExecQuery("SELECT * FROM Win32_PointingDevice")
   var dsc;
   for index,mouse in ..com.each(mouses) {
      dsc = mouse.getDescription();
      if( dsc ? (  ..string.find( dsc,"!\w<@@PS@>/*2\W")  || mouse.getDeviceInterface() == 4  ) 
         && ( ! ..string.find( dsc,"<@@TouchPad@>") ) ){
         ps2Mouse = dsc;  
      }
   } 
   
   var kbs = service.ExecQuery("SELECT * FROM Win32_Keyboard")
   for index,kb in ..com.each(kbs) {
      dsc = kb.getDescription();
      if( ! ..string.startWith(kb.getDeviceID(),"^USB\",true) ){
      	if( dsc ? ..string.find( dsc,"!\w<@@PS@>/*2\W") ){
         	ps2Keyboard = dsc;
         	break;
      	}
      }
   }
   
   if(!ps2Keyboard){
      for( index,kb in ..com.each(kbs) ){
         if( ! ..string.startWith(kb.getDeviceID(),"^USB\",true) ){
            ps2Keyboard = dsc;
            break;
         } 
      } 
   } 
    
   locator,service,mouses,kbs = null;
   collectgarbage("collect");
   return ps2Mouse,ps2Keyboard;
}
 
try{
	ps2Mouse,ps2Keyboard = getPs2Device();
}
catch(e){
	ps2Mouse,ps2Keyboard = null,"PS/2 键盘"
}
mouseIs3d = !!( ( ::GetSystemMetrics( 43/*SM_CMOUSEBUTTONS*/ ) > 2 ) && ::GetSystemMetrics( 75/*_SM_MOUSEWHEELPRESENT*/ ) )
 
/**intellisense(winring0.ps2)
mouseIs3d = 鼠标是否有滚轮，默认自动检测,\n如果系统有多个鼠标请正确设置此值
ps2Mouse = 系统自动检测到的P2鼠标,\n该值为空则禁用鼠标硬件模拟
ps2Keyboard = 系统自动检测到的P2键盘名称,\n该值为空则禁用键盘硬件模拟
getPs2Device() = 检测PS2设备,返回两个值: PS2鼠标设备名,PS2键盘设备名
readMouseID() = 返回鼠标设备ID
waitMouseOutputEmpty() = 读数据以前要确认鼠标输出缓冲区为空
waitMouseOutputFull() = 等待鼠标缓冲区满
waitKeyOutputEmpty() = 等待键盘缓冲区空
waitKeyOutputFull() = 等待键盘缓冲区满
waitInputEmpty() = 在向64h端口写数据之前必须确保Input Register是空的
waitOutputFull() =在向60h端口读取数据之前必须确保Output Register中有数据
waitOutputEmpty() = 等待Output Register空
waitMouseOutputFullAsyn() = 等待鼠标Output Register满\n不论有没有等到按键都返回值，成功返回true，失败返回false
waitKeyOutputFullAsyn() = 等待键盘Output Register满\n不论有没有等到按键都返回值，成功返回true，失败返回false
waitOutputFullAsyn() = 等待Output Register满\n不论有没有等到按键都返回值，成功返回true，失败返回false
enableKb(true) = 禁用键盘，取消禁用参数为false
enableMouse(true) = 禁用鼠标，取消禁用参数为false
writeK60(.(数值) = 向60端口写数据
writeK64(.(数值) = 向64端口写数据
writeScanKey(.(扫描码) = 写入键盘扫描码
writeKeyDown(.(键码) = 写入按下虚拟键码
writeKeyUp(.(键码) = 写入弹起虚拟键码
writeMouse(.(x,y,消息ID) = 写入鼠标消息
enableKbIrq(true) = 启用键盘中断,false为禁用
reakKey(.(是否允许发送键) = 读取按键

delayPress = 更改每次击键并松开后的时间间隔,默认为5毫秒
delayDown = 每次调用key.down函数的默认时间间隔,默认为5毫秒\n在调用press以及pressEx函数时,按下松开键之间也会按此设定延时
delayUp = 每次调用key.up函数后的时间间隔,默认为1毫秒
delay = 指定延时函数

keyDown(.(键名或键盘) = 按下键 
keyUp(.(键名或键盘) = 弹起键 
keyPress(.(键名或键盘) = 按下并弹起键
combine("CTRL","__") = 发送组合热键,参数个数不限.\n参数可以是键名字,或者按键的虚拟码(_VK前缀常量)
mouseMove(.(x,y,是否绝对坐标) = 移动鼠标
mouseDown(.(x,y,是否绝对坐标) = 鼠标左键按下 
mouseUp(.(x,y,是否绝对坐标) = 鼠标左键弹起 
mousePress(.(x,y,是否绝对坐标) = 鼠标左键按下弹起
mouseDownRb(.(x,y,是否绝对坐标) = 鼠标右键按下
mouseUpRb(.(x,y,是否绝对坐标) = 鼠标右键弹起
mousePressRb(.(x,y,是否绝对坐标) = 鼠标右键按下弹起 
mouseDownMb(.(x,y,是否绝对坐标) = 鼠标中键按下 
mouseUpMb(.(x,y,是否绝对坐标) = 鼠标中键弹起 
mousePressMb(.(x,y,是否绝对坐标) = 鼠标中键按下弹起
end intellisense**/

/*intellisense()
winring0.ps2 = PS2鼠标键盘硬件模拟支持库,\n支持win7，但需要管理员权限,\n引用此库发布程序时将自动添加管理员权限到Manifest
end intellisense*/
