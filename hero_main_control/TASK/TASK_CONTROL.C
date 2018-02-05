#include "task_control.h"				 
#define PWM_Period 200
PID_Struct PID_Rammer_moto;
PID_Struct PID_Yaw_V_Struct;			    
PID_Struct PID_Pitch_V_Struct;
PID_Struct PID_Roll_V_Struct;
PID_Struct PID_Yaw_P_Struct;						   
PID_Struct PID_Pitch_P_Struct;
PID_Struct PID_Roll_P_Struct;
PID_Struct Chassis_Position_Control;//����pid
#define moto_max  660
#define moto_min -660
extern RC_Ctl_t RC_Ctl;
extern float angle;
extern volatile unsigned char sbus_rx_buffer[25];
extern float target_angle;
u8 Gear=0,Gear_flag;
int count,temp;
s16 limit_pitch_down,limit_pitch_up,limit_yaw_left,limit_yaw_right;
s16 Rammer_Moto_Measured_Speed,Rammer_Moto_Speed,Set_Rammer_Moto_Speed;//��������ٶ�
s16 Friction_wheel_set_speed;
s32 expect_position[5];

void control_task(void *pdata)
{
			while(1)
			{
				      Rammer_moto_run();//��������ٶȿ���
				      keyboard();
						  if(RC_Ctl.key.v&0x10)
		           Gear_flag=1;
							if(RC_Ctl.rc.s2==2)//��Ħ���ֲ��򿪺���
								{
									LASER_on();
									Friction_wheel_control(Friction_wheel_set_speed,1);//�����ӵ�
									if(RC_Ctl.mouse.press_l==1||RC_Ctl.rc.s1==1)//���
									 Rammer_Moto_Speed=Set_Rammer_Moto_Speed;
									 else
									 Rammer_Moto_Speed=0;		 
								}  
							else 
								{
										LASER_off();
									Rammer_Moto_Speed=0;
										LED_GREEN_OFF();
									Friction_wheel_control(Friction_wheel_set_speed,0);
								}
				if(temp==20)
				{
					 temp=0;
				 if(Rammer_Moto_Measured_Speed==0)
					 count++;
			  }
				if(Gear_flag)
				{
					Gear_flag=0;
					Gear++;
					Gear=Gear%2;
				}
				temp++;
				delay_ms(10);
	    }
}

//Ħ���ֿ���
void Friction_wheel_control (s16 speed,u8 mode)//�����ӵ�
{
	if(mode)
	{
		PWM1=speed;
		PWM2=speed;
	}
	else
	{
		PWM1=1000;
		PWM2=1000;
	}

}


 /****************************
���ܣ�     ���̵���ٶ��޷�
���룺��ǰ�ٶ�
������޷��ٶ�
*****************************/
s16 pwm_confine(s16 Pwm)
 {
	 s16 count;
	 float PID_count;
	 count=(s16)PID_count;
	 if(Pwm>moto_max)
		 return moto_max;
	 else if(Pwm<moto_min)
		 return moto_min;
	 else
	 {	
     if	(Pwm>=0)	 
		 return pow(Pwm,2)/moto_max;
		 else
		 return pow(Pwm,2)/moto_min; 
	 }
 }
 
  /****************************
���ܣ�    ���̲ٿ�
���룺 A��ͨ��1  B��ͨ��2
�������
*****************************/
 void keyboard_scan(s16 *A,s16 *B)
 {
	 #define step 20
	 static u16 num;
	 	 if(RC_Ctl.key.v& KEY_D)  // key: d
	 {
		 *A=*A-step;
	 }
	 	else if(RC_Ctl.key.v& KEY_A) //key: a
	 {
		 *A=*A+step;
	 }

	 	else if(RC_Ctl.key.v & KEY_W)  // key: w
	 {
		 *B=*B+step;
	 }
	 	else if(RC_Ctl.key.v& KEY_S) //key: s
	 {
		 *B=*B-step;
	 }
	else
		{
            num++;
			if(num>20)
			{
				num=0;
				*A=0;
				*B=0;
			}
		}
     if(*A>moto_max)
		 {
			 *A=moto_max;
		 }
		 else if(*A<moto_min)
		 {
			 *A=moto_min;
		 }
		  if(*B>moto_max)
		 {
			 *B=moto_max;
		 }
		 else if(*B<moto_min)
		 {
			 *B=moto_min;
		 }
 }
 //����ɨ��
 void keyboard(void)
 {
	 key_a=keyboard_data(KEY_A);
	 key_d=keyboard_data(KEY_D);
	 key_w=keyboard_data(KEY_W);
	 key_s=keyboard_data(KEY_S);
	 key_z=keyboard_data(KEY_Z);
	 key_x=keyboard_data(KEY_X);
	 key_c=keyboard_data(KEY_C);
	 key_v=keyboard_data(KEY_V);
	 key_g=keyboard_data(KEY_G);
	 key_f=keyboard_data(KEY_F);
	 key_r=keyboard_data(KEY_R);
	 key_e=keyboard_data(KEY_E);
	 key_q=keyboard_data(KEY_Q);
	 key_b=keyboard_data(KEY_B);
	 key_ctrl=keyboard_data(KEY_CTRL);
	 key_shift=keyboard_data(KEY_SHIFT);
	 key_jump_a=keyboard_jump_1(key_q);
	 key_jump_d=keyboard_jump_2(key_e);
 }
//����ɨ���Ӻ���   �㷨��ʵ�ֹ��ܵ����д�����
//(�м���key��Ҫ�����������ĺ�����ʹ�������ӷ��,����ʵ�ֵ���һ������������м��)ԭ������static�����ܲٵ������ֲ���
//////////////////////////////////
 u8 keyboard_data(u16 key)
 {
	 if(RC_Ctl.key.v& key)
		 return 1;
	 else
		 return 0;
 }
 u8 keyboard_jump_1(u8 key)
 {
	 static u8 key_falg,key_value,last_key_value;
	 static u32 temp;

		last_key_value=key_value;
		key_value=key;
		if(last_key_value==0&&key_value==1) 
		{
			temp++;
		if(temp%2)
		 key_falg=0;
	 else
		 key_falg=1;
		}
	 return key_falg;
 }
 
u8 keyboard_jump_2(u8 key)
 {
	 static u8 key_falg,key_value,last_key_value;
	 static u32 temp;

		last_key_value=key_value;
		key_value=key;
		if(last_key_value==0&&key_value==1) 
		{
			temp++;
		if(temp%2)
		 key_falg=0;
	 else
		 key_falg=1;
		}
	 return key_falg;
 }
///////////////////////////////////////////////////////// 
  /****************************
���ܣ�      �������к����㷨
���룺 ��
�������
*****************************/
 void moto_ctrl()
 {
	 s16 d;
	 d=RC_Ctl.rc.ch1-1024;
	 pwm_set_1=d;
	 pwm_set_2=d;
	 pwm_set_3=d;
	 pwm_set_4=d;
 }
void Position_control()
{
	s16 position_out[5];
	position_out[1]=PID_Calculate(&Chassis_Position_Control,measured_Position[1],expect_position[1]);
	position_out[2]=PID_Calculate(&Chassis_Position_Control,measured_Position[2],expect_position[2]);
	position_out[3]=PID_Calculate(&Chassis_Position_Control,measured_Position[3],expect_position[3]);
	position_out[4]=PID_Calculate(&Chassis_Position_Control,measured_Position[4],expect_position[4]);
	V_CTIM_Driver_Speed(CAN1, position_out[1], position_out[2], position_out[3], position_out[4]);
}	
/*************************
      ��ʼ��PID����
*************************/
void PID_Init(PID_Struct *PID)
{
  PID->expectation       = 0.0;            //ң�ظ�������ֵ
  PID->Err_k			       = 0.0;            //��ǰ���ֵe(k)
  PID->Err_k_1		       = 0.0;            //k-1ʱ�����ֵe(k-1)
  PID->Err_k_2		       = 0.0;            //k-2ʱ�����ֵe(k-2)
  PID->SumErr              = 0.0;		//����
  PID->Kp				   = 0.0;           //����ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Ti				   = 0.0;           //����ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Td				   = 0.0;           //΢��ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Ouput_deltaUk       = 0.0;		    //PID�����������U(k) - U(k-1)
  PID->Ouput_deltaUk_Max   = PWM_Period;		    //������������ֵ
  PID->Ouput_deltaUk_Min   = -PWM_Period;		    //�����������Сֵ
  PID->PID_Integral_Max    = 100.0;		     //���ƻ��������ֵ
  PID->PID_Integral_Min    = -100.0;			//���ƻ�������Сֵ
}
void PID_GimBal_Init(PID_Struct *PID)
{
  PID->expectation       = 0.0;            //ң�ظ�������ֵ
  PID->Err_k			       = 0.0;            //��ǰ���ֵe(k)
  PID->Err_k_1		       = 0.0;           //k-1ʱ�����ֵe(k-1)
  PID->Err_k_2		       = 0.0;           //k-2ʱ�����ֵe(k-2)
  PID->SumErr              = 0.0;			//����
  PID->Kp				   = 0.0;           //����ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Ti				   = 0.0;           //����ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Td				   = 0.0;           //΢��ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Ouput_deltaUk       = 0.0;		    //PID�����������U(k) - U(k-1)
  PID->Ouput_deltaUk_Max   = 5000;		    //������������ֵ
  PID->Ouput_deltaUk_Min   = -5000;		    //�����������Сֵ
  PID->PID_Integral_Max    = 4000.0;		     //���ƻ��������ֵ
  PID->PID_Integral_Min    = -4000.0;			//���ƻ�������Сֵ
}
void PID_Rammer_moto_Init(PID_Struct *PID)
{
  PID->expectation         = 0.0;           //ң�ظ�������ֵ
  PID->Err_k			   = 0.0;           //��ǰ���ֵe(k)
  PID->Err_k_1		       = 0.0;           //k-1ʱ�����ֵe(k-1)
  PID->Err_k_2		       = 0.0;           //k-2ʱ�����ֵe(k-2)
  PID->SumErr              = 0.0;			//����
  PID->Kp				   = 5;           //����ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Ti				   = 1;           //����ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Td				   = 0.0;           //΢��ϵ����ͨ���������ߵ�PID������д��Flash
  PID->Ouput_deltaUk       = 0.0;		    //PID�����������U(k) - U(k-1)
  PID->Ouput_deltaUk_Max   = 1000;		    //������������ֵ
  PID->Ouput_deltaUk_Min   = -1000;		    //�����������Сֵ
  PID->PID_Integral_Max    = 1000.0;		//���ƻ��������ֵ
  PID->PID_Integral_Min    = -1000.0;		//���ƻ�������Сֵ
}

void pid_init_struct(void)
{
	PID_Rammer_moto_Init(&PID_Rammer_moto);
	PID_Init(&Chassis_Position_Control);
	
}

/***************************************************************
PID����-����ʽ
***************************************************************/
float PIDz_Calculate(PID_Struct* PID, float measured, float expect)
{
     float K_p = PID->Kp;
     float T_d = PID->Td;
     float T_i = PID->Ti;
     float increment;
    PID->Err_k=PID->Err_k_1;
    PID->Err_k_1=PID->Err_k_2;
    PID->Err_k_2=(expect-measured)*100;
    increment = (K_p+T_i+T_d)*PID->Err_k_2-(K_p+2*T_d)*PID->Err_k_1+T_d*PID->Err_k;
    PID->Ouput_deltaUk+=increment;
    if(PID->Ouput_deltaUk > PID->Ouput_deltaUk_Max)
    {
        PID->Ouput_deltaUk = PID->Ouput_deltaUk_Max;
    }
    if(PID->Ouput_deltaUk < PID->Ouput_deltaUk_Min)
    {
        PID->Ouput_deltaUk = PID->Ouput_deltaUk_Min;
    }
    
  return PID->Ouput_deltaUk;
}

/***************************************************************
PID����-λ��ʽ
***************************************************************/
float PID_Calculate(PID_Struct* PID, float measured, float expect)
{
  float Value_Proportion;  
  float Value_Integral;		
  float Value_Derivative;	

  PID->expectation =  expect;
  PID->Err_k = (PID->expectation - measured)*10;
  PID->SumErr+= PID->Err_k;

  //P I D
  Value_Proportion    = PID->Kp * PID->Err_k;
  Value_Integral      =  PID->SumErr * PID->Ti;	
  Value_Derivative  = PID->Kp * PID->Td * (PID->Err_k - PID->Err_k_1);

  if(Value_Integral > PID->PID_Integral_Max)
  {
    PID->SumErr -= PID->Err_k;
	Value_Integral = PID->PID_Integral_Max;
  }
  if(Value_Integral < PID->PID_Integral_Min)
  {
  	PID->SumErr -= PID->Err_k;
    Value_Integral = PID->PID_Integral_Min;
  }
  
  PID->Ouput_deltaUk = Value_Proportion + Value_Integral + Value_Derivative;

  if(PID->Ouput_deltaUk > PID->Ouput_deltaUk_Max)
  {PID->Ouput_deltaUk = PID->Ouput_deltaUk_Max;}
  if(PID->Ouput_deltaUk < PID->Ouput_deltaUk_Min)
  {PID->Ouput_deltaUk = PID->Ouput_deltaUk_Min;}

  PID->Err_k_1 = PID->Err_k;	  //����k-1�����ֵ
  
  return PID->Ouput_deltaUk;
}