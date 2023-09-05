#include<stdio.h>
#include <string>
using namespace std; 
int X0,X1,X2,X3,X4,insert,select,power = 1;
int X10,X11,X12,X13,X14;
int X20,X21,X22,X23;
int X30,X31,X32,X33,X34,X35,X36,res;
int X40,X41,X42,X43,X44,approved;
char* str="";

void grafcet0();
void grafcet1();
void grafcet2();
void grafcet3();
void grafcet4();
void grafcet4();
void active0();

void grafcet0() 
{
	if(X0 == 1)
	{
		X0 = 0;
		X1 = 1;
		X10 = 1;
		return;
	}
	if(X1 == 1 && X14 == 1) 
	{
		X1 = 0;
		X14 = 0;
		X2 = 1;
		X20 = 1;
		return;
	}
	if(X2 == 1 && (X22 ==1 || X23 ==1))
	{
		X2 = 0;
		X22 = 0;
		X23 = 0;
		X3 = 1;
		X30 = 1;
		return;
	}
	if(X3 == 1 && X36 == 1)
	{
		X3 = 0;
		X36 = 0;
		return;
	}
	active0();
//	if (str!= ""){ 
//		printf("\n");
//		printf("%s\n",str);
//		str = "";
//	}
}
void grafcet1(){
	if(X10 == 1 &&  X1== 1) {
		X10 = 0;
		X11 = 1;	
	}
	else if(X11 == 1 & insert == 1){
		X11 = 0;
		X12 = 1;
	}
	else if(X11 == 1 & select == 1){
		X11 = 0;
		select = 0;
		X13 = 1;
	}
	else if(X12 == 1){
		X12 = 0;
		X14 = 1;
		printf("\n");
		str = "AES資料加密\n";
		printf("%s\n",str);
	}
	else if(X13 == 1){
		X13 = 0;
		X14 = 1;
		printf("\n");
		str= "AES資料解密\n";
		printf("%s\n",str);
	}
	else if(X14 == 1){
		X14 = 0;
		X10 = 1;
	}
	
	
}
void grafcet2(){
	if(X2 == 1 &&  X20== 1) {
		X20 = 0;
		X21 = 1;
	}
	else if(X21 == 1 && insert == 1){
		X21 = 0;
		X22 = 1;
		printf("\n");
		str = "insert data\n";
		printf("%s\n",str);
	}
	else if(X21 == 1 && select == 1){
		X21 = 0;
		X23 = 1;
		printf("\n");
		str= "select data\n";
		printf("%s\n",str);
	}
	else if(X22 == 1 || X23 == 1){
		X22 = 0;
		X23 = 0;
		X20 = 1;
	}
}
void grafcet3(){

	if(X3 == 1 &&  X30== 1) {
		X30 = 0;
		X31 = 1;
	}
	else if(X31 == 1){
		X31 = 0;
		X32 = 1;
		printf("\n");
		str = "檔案系統初始化\n";
		printf("%s\n",str);
		printf("FR_OK = ");
		scanf("%d", &res);
	}
	else if(X32 == 1 && res == 1){
		X32 = 0;
		X34 = 1;
		printf("\n");
		str = "開啟文件\n";
		printf("%s\n",str);
	}
	else if(X32 == 1 && res == 0){
		X32 = 0;
		X33 = 1;
		printf("\n");
		str = "建立文件\n";
		printf("%s\n",str);
	}
	else if(X33 == 1 || X34 ==1){
		X33 = 0;
		X34 = 0;
		X35 = 1;
		printf("\n");
		str = "讀取文件資料\n";
		printf("%s\n",str);
	}
	else if(X35 == 1){
		X36 = 1;
		X35 = 0;
		printf("\n");
		str = "卸載文件系統\n";
		printf("%s\n",str);
	}
	else if(X36 == 1){
		X36 = 0;
		X30 = 1;
	}
}

void grafcet4(){
	if(X4 == 1){
		X4 = 0;
		X41 = 1;
	}
	else if(X41 == 1){
		X41 = 0;
		X42 = 1;
		printf("\n");
		str = "Wifi連線\n";
		printf("%s",str);
		str = "伺服器環境初始化\n";
		printf("%s\n",str);
	}
	else if (X42 == 1){
		printf("approved = ");
		scanf("%d", &approved);
		if(approved == 1){
			X42 = 0;
			X43 = 1;
		}
		else{
			X42 = 1;
		}
	}
	else if (X43 == 1){
		X43 = 0;
		X44 = 1;
		printf("\n");
		str = "網頁遠端監控\n";
		printf("%s\n",str);
	}
	else if (X44 == 1){
		X44 = 0;
		X40 = 1; 
	} 
	printf("X40 = %d X2=41 = %d X42 = %d X43 = %d X44 = %d \n",X40,X41,X42,X43,X44); 
}
void active0()
{
	if(X1 == 1) 
		grafcet1(); //資料加密 	
	else if(X2 == 1)
		grafcet2(); //資料庫系統 
	else if(X3 == 1)
		grafcet3(); //檔案系統 
	else if(X4 == 1)
		grafcet4(); //網頁伺服器 

}
int main(void)
{
	printf("insert = ");
	scanf("%d", &insert);
	printf("select = ");
	scanf("%d", &select);

	if(insert == 1){
		X0 = 1;
	}
	if(select == 1){
		X4 = 1;
	}
	while(power == 1)
	{
		grafcet0();
		grafcet4();
		printf("X0 = %d X1 = %d X2 = %d X3 = %d X4 = %d ",X0,X1,X2,X3,X4); 
		printf("X10 = %d X11 = %d X12 = %d X13 = %d X14 = %d ",X10,X11,X12,X13,X14); 
		printf("X20 = %d X21 = %d X22 = %d X23 = %d ",X20,X21,X22,X23); 
		printf("X30 = %d X2=31 = %d X32 = %d X33 = %d X34 = %d  X35 = %d  X36 = %d ",X30,X31,X32,X33,X34,X35,X36);
	}
}
