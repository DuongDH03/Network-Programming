#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct student {
   char name[20];
    int eng;
    int math;
    int phys;
    double mean;
}STUDENT;

STUDENT *p;

STUDENT data[] = {
    {"Tuan", 82, 72, 58, 0.0},
    {"Nam", 77, 82, 79, 0.0},
    {"Khanh", 52, 62, 39, 0.0},
    {"Phuong", 61, 82, 88, 0.0}
};

double meanCal(struct student s) {
    return (s.eng + s.math + s.phys)/3;
}

void ranking(double x) {
    if (x >= 90 && x <= 100)
        printf("Grade: S\n");
    else if (x >= 80 && x < 90)
            printf("Grade: A\n");
        else if (x >= 70 && x < 80)
            printf("Grade: B\n");
            else if (x >= 60 && x <70 )
                printf("Grade: C\n");
                else printf("Grade: D\n");
}

int main() {
    p = data;
    printf("Student Grade:\n");
    for (int i = 0; i < 4; i++) {
        p->mean = meanCal(*p);
        printf("%s\t%lf\t",p->name, p->mean);
        ranking(p->mean);
        p++;
    }    

    return 0;
}