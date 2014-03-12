#include "bootpack.h"

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	//等待键盘控制电路准备完毕
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void)
{
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	//如果激活成功，会有一个ACK(1B 0xfa)发送回来
	return; 
}

//encapsulate processing of checker->mouseCheckBuffer and mouseChecker
void init_MouseChecker(struct MouseChecker *checker)
{
	checker->phase = 0;
	return;
}

int MouseChecker_Job(struct MouseChecker *checker, int i)
{
	if (checker->phase == 0)
	{
		if (i == 0xfa)
		{
			checker->phase = 1;
		}
		goto pending;
	}else if(checker->phase == 1)
	{
		if((i & 0xc8) == 0x08){
			checker->mouseCheckBuffer[0] = i;
			checker->phase = 2;
		}
			goto pending;
	}
	else if(checker->phase == 2)
	{	
			checker->mouseCheckBuffer[1] = i;
			checker->phase = 3;
			goto pending;

	}else if(checker->phase == 3){
		checker->mouseCheckBuffer[2] = i;
		checker->phase = 1;
		checker->btn = checker->mouseCheckBuffer[0] & 0x07;
		checker->x = checker->mouseCheckBuffer[1];
		checker->y = checker->mouseCheckBuffer[2];
		if ((checker->mouseCheckBuffer[0] & 0x10) != 0) {
			checker->x |= 0xffffff00;
		}
		if ((checker->mouseCheckBuffer[0] & 0x20) != 0) {
			checker->y |= 0xffffff00;
		}
		checker->y = checker->y;
		goto done;
	}

error: 
	return -1;
done:
	return 1;
pending:
	return 0;
}

