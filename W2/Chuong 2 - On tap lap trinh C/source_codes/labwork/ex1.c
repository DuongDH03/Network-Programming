#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct student {
    char name[20];
    int eng;
    int math;
    int phys;
    double mean;
};

static struct student data[]= {
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

    printf("Student Grade:\n");
    for (int i = 0; i < 4; i++) {
        data[i].mean = meanCal(data[i]);
        printf("%s\t%lf\t",data[i].name, data[i].mean);
        ranking(data[i].mean);
    }    

    return 0;
}