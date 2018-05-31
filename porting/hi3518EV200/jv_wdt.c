
#include "jv_common.h"
#include "jv_wdt.h"

struct watchdog_info{
    unsigned int options;   //options the card/driver supprots 19         
    unsigned int firmware_version;  //firmcard version of the card
    unsigned char identity[32];     //identity of the board 21
 };

#define WATCHDOG_IOCTL_BASE 'W'
#define WDIOC_GETSUPPORT _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define WDIOC_SETTIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT _IOR(WATCHDOG_IOCTL_BASE, 7, int)
#define WDIOS_DISABLECARD 0x0001        /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD 0x0002 /* Turn on the watchdog timer */
#define WDIOC_SETOPTIONS _IOR(WATCHDOG_IOCTL_BASE, 4, int)
#define WDIOC_KEEPALIVE _IOR(WATCHDOG_IOCTL_BASE, 5, int)

/**
*@brief �����Ź���Ϊ�˲���û��Ӳ���Ź�,lck20120616
*
*/
static PJDOG pDog;

/**
 *@brief �򿪿��Ź�
 *@note ���������Ȱ�װ
 *@return �ɹ������ش򿪵��豸�����ʧ�ܷ��� -1
 *
 */
HWDT jv_open_wdt()
{
	//��ʼ�������Ź�
	S32 segment_id = shmget(0x80040001, sizeof(JDOG), S_IRUSR|S_IWUSR);
	if (segment_id != -1)
	{
		pDog = shmat(segment_id, 0, 0);
		if (pDog)
		{
			pDog->nInitOK = TRUE;
		}
	}
	else
	{
		Printf("No WatchDog Found!\n");
	}

	//��ʼ��Ӳ���Ź�
	HWDT iDog = 0;
	iDog = open("/dev/watchdog", O_RDONLY);
	if (iDog == -1) 
	{
		static BOOL haveWDT = TRUE;
		if (haveWDT)
		{
			if (access("/home/ipc_drv/extdrv/wdt.ko", F_OK) == 0)
			{
				utl_system("insmod /home/ipc_drv/extdrv/wdt.ko default_margin=60 nodeamon=1");
				//���ﲻ���ٴ������´���������Ȼ�ʹ���
			}
			int i;
			for (i=0;i<5;i++)
			{
				//�в��ɹ�����������Լ���
				iDog = open("/dev/watchdog", O_RDONLY);
				if (iDog != -1)
				{
					break;
				}
				Printf("Failed one time, Retry...\n");
				usleep(300*1000);
			}
			if (iDog == -1)
			{
				haveWDT = FALSE;
				perror("No KO, fail to open wdt!  /home/ipc_drv/extdrv/wdt.ko");
				return -1;
			}
		}
		else
		{
			perror("fail to open wdt!");
			return -1;
		}
	}

	S32 nOption = 30;
	ioctl(iDog, WDIOC_SETTIMEOUT, &nOption);
	ioctl(iDog, WDIOC_GETTIMEOUT, &nOption);
	Printf("Watchdog=%d enabled, timeout=%d\n", iDog, nOption);

	nOption = WDIOS_ENABLECARD;
	ioctl(iDog, WDIOC_SETOPTIONS, &nOption);

	return iDog;
}

/**
 *@brief �رտ��Ź�
 *@param S32 iDog �Ѵ򿪵��豸���
 */
void jv_close_wdt(HWDT iDog)
{
	if (pDog)
	{
		pDog->nQuit = TRUE;
		shmdt(pDog);
	}
	if(iDog > 0)
	{
		Printf("--------------------------close watchdog \n");
		S32 nOption = WDIOS_DISABLECARD;
		ioctl(iDog, WDIOC_SETOPTIONS, &nOption);
		close(iDog);
	}
	return;
}

/**
 *@brief ι��
 *@param S32 iDog �Ѵ򿪵��豸���
 */
void jv_feed_wdt(HWDT iDog)
{
	//Printf("--------------------------feeding watchdog \n");
	if (pDog)
	{
		pDog->nCount = 0;
	}
	if (iDog > 0)
	{
		//Printf("feed watch dog...\n");
		ioctl(iDog, WDIOC_KEEPALIVE, NULL);
	}
	return;
}
