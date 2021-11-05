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

void swapf(double *a, double *b){
    double t = *a;
    *a = *b;
    *b = t;    
}

//Алгоритм сортирующей строки
void triangle(tgaImage *image,
              int x0, int y0, double z0, double u0, double v0,
              int x1, int y1, double z1, double u1, double v1,
              int x2, int y2, double z2, double u2, double v2, double zbuffer[image->width][image->height],
              double intensity, Model *model)
{
    tgaColor color;
    // Sorting vertices by 'y' coord
    if (y0 > y1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
        swapf(&z0, &z1);
    //    swapf(&u0, &u1);
   //     swapf(&v0, &v1);
    }
    if (y0 > y2) {
        swap(&x0, &x2);
        swap(&y0, &y2);
        swapf(&z0, &z2);
    //    swapf(&u0, &u2);
    //    swapf(&v0, &v2);
    }
    if (y1 > y2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
        swapf(&z1, &z2);
     //   swapf(&u1, &u2);
    //    swapf(&v1, &v2);
    }
 
    double tless, tmore, tz, z, za, zb;
    double xleft, xright, x;
    // For texture
//    double ua, ub, va, vb, u, v;
 //   unsigned int h = model->diffuse_map->height;
 //   unsigned int w = model->diffuse_map->width;
    int r, g, b;
     
    for(int y = y0; y <= y2; y++){
        // Finding boundary coords using the parametric equation of the line
        tmore = (double)(y - y0) / (y2 - y0);
        if(y > y1){
            tless = (double)(y - y1) / (y2 - y1);
            xleft = x2 * tless + x1 * (1 - tless);
            za = z2 * tless + z1 * (1 - tless);
      //      ua = u2 * tless + u1 * (1 - tless);
      //      va = v2 * tless + v1 * (1 - tless);
        }
        else{
            tless = (double)(y - y0) / (y1 - y0);
            xleft = x1 * tless + x0 * (1 - tless);
            za = z1 * tless + z0 * (1 - tless);
    //        ua = u1 * tless + u0 * (1 - tless);
    //        va = v1 * tless + v0 * (1 - tless);
        }
        xright = x2 * tmore + x0 * (1 - tmore); 
        zb = z2 * tmore + z0 * (1 - tmore);
     //   ub = u2 * tmore + u0 * (1 - tmore);
    //    vb = v2 * tmore + v0 * (1 - tmore);
        if (xleft > xright){
            swapf(&xleft, &xright);
        }
        if (za > zb){
            swapf(&za, &zb);
        }
      //  if (ua > ub){
     //       swapf(&ua, &ub);
    //    }
    //    if (va > vb){
    //        swapf(&va, &vb);
    //    }
        x = xleft;
        // Scan line for x
        while(x <= xright){
            tz = (double)(x - xleft) / (xright - xleft);
            z = zb * tz + za * (1 - tz);
            // Checking z-buffer
            if(z > zbuffer[(int)x][y]){
            //    u = ub * tz + ua * (1 - tz);
            //    v = vb * tz + va * (1 - tz);
                zbuffer[(int)x][y] = z;
           //     color = tgaGetPixel(model->diffuse_map, w * u, h * v);
              r = Red(255) * intensity;
               g = Green(255) * intensity;
               b = Blue(255) * intensity;
                tgaSetPixel(image, x, y, tgaRGB(r, g, b));
            }
            x += 1;
        }
    }
}


//Процедура создания полигоной модели с простым освещением
void meshgrid(tgaImage *image, Model *model){
    double light[3] = {1,0, 0};
    double a[3], b[3], n[3], leng, intensity;
    int screen_coords[3][2];
    Vec3 *v[3];
    Vec3 *s[3];
    double zbuffer[image->width][image->height];
       for(int i=0; i<model->nface;++i){  //Перевод в экранные координаты		 
		for(int j=0; j<3; ++j){
			v[j] = getVertex(model,i,j); 
         //   s[j] = getDiffuseUV(model, i, j);
            screen_coords[j][0] = ((*v[j])[0]/4000+1.0)*(image->width)/2;
            screen_coords[j][1] = ((*v[j])[1]/4000+1.0)*(image->height)/2;
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
        //zbuffer содержит минимальные значения
          for(int i = 0; i < (image->width); i++){
        for (int j = 0; j < (image->height); j++){
            zbuffer[i][j] = -1000000;
        }
          }  
       if(intensity <= 0){
          triangle(image, screen_coords[0][0], screen_coords[0][1], (*v[0])[1], (*s[0])[0], (*s[0])[1],
                        screen_coords[1][0], screen_coords[1][1], (*v[1])[1]/4000, (*s[1])[0], (*s[1])[1],
                        screen_coords[2][0], screen_coords[2][1], (*v[2])[1]/4000, (*s[2])[0], (*s[2])[1], zbuffer,
                        fabs(intensity), model);
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
    int height = 4000;
    int wigth = 4000;
	image = tgaNewImage(wigth, height, RGB); 
	if(!image){
		perror("tgaNewImage");
	}
	//loadDiffuseMap(model, argv[2]);
	meshgrid(image,model);
	tgaSaveToFile(image, "render_karambit.tga");
	tgaFreeImage(image);
	freeModel(model);
	return 0;
}