#include <stdio.h>
#include <math.h>
#include "tga.h"
#include "model.h"

//----------------Процедуры------------

//Процедура смены осей местами
void swap( int* x,  int* y){
	int t = *x;
	*x = *y;
	*y = t;
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


//Процедура создания сетчатой модели
void meshgrid(tgaImage *image, Model *model){
    int i, j; tgaColor color = tgaRGB(0, 255, 0);
	for( i=0; i<model->nface;++i){
		 int screen_coords[3][2]; //Перевод в экранные координаты			
		for(j=0; j<3; ++j){
			Vec3 *v = &(model->vertices[model->faces[i][3*j]]); 
            screen_coords[j][0] = ((*v)[0] + 0.3) * image->width /(2*0.3);
            screen_coords[j][1] = (0.3 - (*v)[1]) * image->height /(2*0.3);
		} 
		//Отрисовка 3-ех ребер
		for (j = 0; j < 3; ++j) {
     line(image,screen_coords[j][0], screen_coords[j][1], screen_coords[(j+1)%3][0], screen_coords[(j+1)%3][1],color);
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
    int height = 1000;
    int wigth = 1000;
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