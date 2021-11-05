#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tga.h"
#include "model.h"

//----------------Процедуры------------

//Процедура смены местами
void swap( int* x,  int* y){
	int temp = *x;
	*x = *y;
	*y = temp;
}

//Процедура целочисленного алгоритма Брезенхема
void line(tgaImage *image,  int x0,  int y0,  int x1,  int y1, tgaColor color){
   
    int sign(int y) { // Функция, возврающающая знак переменной
        if (y > 0) {
            return 1;
        } 
        else if (y == 0) {
            return 0;
        } 
        else {
            return -1;
        }
    }
//Начало алгоритма 
	int steep = 0;			
	if(abs(y1-y0) > abs(x1-x0)){ 	// если угол наклона крутой, то меняем оси местами
		steep = 1;
		swap(&x0, &y0);				
		swap(&x1, &y1);
	}
	if(x0>x1){	// если концевая точка левее начальной, то меняем их местами
		swap(&x0,&x1);		
		swap(&y0,&y1);
	}
    int x;
	int dx = x1 - x0;   
	int dy = y1 - y0;
	float derror = 2*abs(dy);
	float error = 0;
	int y = y0;
	
	for(x=x0; x<=x1; ++x){  //генерация отрезка
		if(steep){
			tgaSetPixel(image, y, x, color);
		} 
        else {
			tgaSetPixel(image, x, y, color);
		}
        error += derror;
		if(error > dx){ 
			y +=sign(y1-y0); //направление роста коорднаты y
			error -= 2*dx;
		}
    }
}

//Алгоритм сортирующей строки
void triangle (tgaImage *image,  int x0,  int y0,  int x1,  int y1, int x2,  int y2, tgaColor color) {
    if (y0>y1) {          //упорядочивание вершин по координате y:y0<=y1 пузырьковом методом
        swap(&x0,&x1);   //у треугольника 3 точки - 0,1,2
        swap(&y0,&y1);
    }
    if (y0>y2) {
        swap(&x0,&x2);
        swap(&y0,&y2);
    }
    if (y1>y2) {
        swap(&x1,&x2);
        swap(&y1,&y2);
    }
 
    double dx02 = 0, dx01 = 0, dx12 = 0; //Вычисляем приращения по оси х для 3ех сторон треугольника
     
    if (y2 != y0) {     
        dx02 = x2 - x0;
        dx02 /= y2 - y0;
    }
    else {
        dx02 = 0;   //Если ординаты 2ух точек совпадает, то приращение полагается 0
    }
    
    if (y0 != y1) {
        dx01 = x1-x0;
        dx01 /= y1 - y0;
    }
    else {
        dx01 = 0;
    }
    
    if (y1 != y2) {
        dx12 = x2 - x1;
        dx12 /= y2 - y1;
    }
     else {
        dx12 = 0;
    }
    
     double wx0 = x0;  // wx0 и wx1 -рабочие точки,между которыми рисуется отрезок- закрашивается треугольник
     double wx1 = wx0; 
     double _dx02 = dx02;  //Сохраняем приращение 02, чтобы использовать для нижнего полутреугольника
 //ВЕРХНИЙ ПОЛУТРЕУГОЛЬНИК   
    if (dx02 > dx01) {  // упорядочиваем приращения так, чтобы wx0 была левее wx1
        double temp = dx01;
        dx01 = dx02;
        dx02 = temp;
    }
    
    for (int i = y0; i < y1; i++) {  //растеризация верхнего полутреугольника по координате y
        for (int j = wx0; j <= wx1; j++) { // рисуем горизонтальную линию между рабочими точками 
            tgaSetPixel(image, j, i, color);
        }
        wx0 += dx02;
        wx1 += dx01;
    }
    
    if (y0==y1){  //при у0=у1 отсутствует верхний полутреугольник
        wx0 = x0;  //переносим вручную точки для растеризации нижнего полутреугольника т.к. они совпадают
        wx1 = x1;
    }
  //НИЖНИЙ ПОЛУТРЕУГОЛЬНИК  
     if (_dx02 < dx12) {  // упорядочиваем приращения так, чтобы wx0 была левее wx1
        double temp = dx12;
        dx12 = _dx02;
        _dx02 = temp;
    }
    
    for (int i = y1; i <= y2; i++) { //растеризация нижнего полутреугольника по координате y
        for (int j = wx0; j <= wx1; j++) { // рисуем горизонтальную линию между рабочими точками 
            tgaSetPixel(image, j, i, color);
        }
        wx0 += _dx02;
        wx1 += dx12;
    }
}


//Процедура создания полигоной модели с простым освещением
void meshgrid(tgaImage *image, Model *model){
    double light[3] = {1,0, 0};
    double a[3], b[3], n[3], leng, intensity;
    int screen_coords[3][2];
    Vec3 *v[3];
       for(int i=0; i<model->nface;++i){  //Перевод в экранные координаты		 
		for(int j=0; j<3; ++j){
			v[j] = getVertex(model,i,j); 
            screen_coords[j][0] = ((*v[j])[0] + 0.2) * image->width /(2*0.3);
            screen_coords[j][1] = (0.2 - (*v[j])[1]) * image->height /(2*0.3);
		} 
           
        //Расчитываем координаты ax,ay,az и bx,by,bz
        for(int i=0; i < 3; ++i){
            a[i] = (*v[1])[i] - (*v[0])[i];  //a= P1-P0 
            b[i] = (*v[2])[i] - (*v[0])[i];  //b=P2-P0
        }

        // координаты нормали к плоскости n=a x b
        n[0] = a[1] * b[2] - a[2] * b[1];     //aybz-azby
        n[1] = (a[0] * b[2] - a[2] * b[0]);  //-1*(axbz-azbx)
        n[2] = a[0] * b[1] - a[1] * b[0];     //axby-aybx

        // Нормализация
        leng = sqrt(n[0] * n [0] + n[1] * n [1] + n[2] * n [2]);
        n[0] = n[0]/leng;
        n[1] = n[1]/leng;
        n[2] = n[2]/leng;
           
        // I=l*normalize.n 
        intensity = light[0] * n[0] + light[1] * n[1] + light[2] * n[2];
           
       if(intensity <= 0){
        triangle(image, screen_coords[0][0], screen_coords[0][1],  //закраска 
                        screen_coords[1][0], screen_coords[1][1],
                        screen_coords[2][0], screen_coords[2][1],
                        tgaRGB((int)(fabs(intensity) * 255), (int)(fabs(intensity) * 255), (int)(fabs(intensity) * 255)));
       }
    }
}

//--------------Основная программа-------------
int main(int argc, char *argv[]){
    Model *model = NULL;
    tgaImage *image = NULL;
    
    	model = loadFromObj(argv[1]);	
	if(!model){
		perror("loadFromObj");
	}
    int height = 800;
    int wigth = 800;
	image = tgaNewImage(wigth, height, RGB); 
	if(!image){
		perror("tgaNewImage");
	}
	
	meshgrid(image,model);
	tgaSaveToFile(image, "karanbit.tga");
	tgaFreeImage(image);
	freeModel(model);
	return 0;
}