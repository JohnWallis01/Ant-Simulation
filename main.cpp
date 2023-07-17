#include <cmath>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SFML/Graphics.hpp>



//constants
#define M_PI 3.14159265358979323846  /* pi */

//Simulation Parameters
#define Number_Ants 20
#define Canvas_X 1280
#define Canvas_Y 720
#define Number_Foods 500

//Ant States
#define Ant_Foraging 0
#define Ant_Homing 1

//Ant Parameters
#define Ant_View_Range 20
#define Ant_Sense_Range 0
#define Ant_View_Angle 0.4 //Radians
#define Ant_Interact_Range 5                           
#define Ant_Movement_Range 1


//logic
#define Type_Food 0
#define Type_Trail 1
#define Type_Colony 2

//ObjectStructs:
struct food_struct {
    struct food_struct* next;
    struct food_struct* last;
    double xpos;
    double ypos;
    struct ant_struct* carrying;
};

struct trail_struct{
    struct trail_struct* next;
    double xpos;
    double ypos;
    double lifetime;

};

struct ant_struct{
    struct ant_struct* next;
    double xpos;
    double ypos;
    double angle;
    char state;
    struct food_struct* carrying;
};

struct colony_struct {
    struct colony_struct* next;
    double xpos;
    double ypos; 
    double radius;
};


struct setup_struct{
    struct ant_struct*      ants_list;
    struct food_struct*     foods_list;
    struct trail_struct*    foragings_list;
    struct trail_struct*    homings_list;
    struct colony_struct*   colonys_list;
};

//MathHelpers:
double isValueInRange(double value, double xbound1, double xbound2) {
    // Check if xbound1 and xbound2 are in numerical order
    if (xbound1 < xbound2) {
        // Check if value is between xbound1 and xbound2
        if (value >= xbound1 && value <= xbound2) {
            return 1;  // Value is in the range
        }
    } else {
        // Check if value is between xbound2 and xbound1
        if (value >= xbound2 && value <= xbound1) {
            return 1;  // Value is in the range
        }
    }
    
    return 0;  // Value is not in the range
}

double randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

//DataInits:
struct food_struct* init_food(struct food_struct** Foods_List, double xpos, double ypos) {
    
    if(*Foods_List == NULL) {
        *Foods_List = (struct food_struct*)malloc(sizeof(struct food_struct)); 
        (*Foods_List)->next = NULL;
        (*Foods_List)->last = NULL;
        (*Foods_List)->xpos = xpos;
        (*Foods_List)->ypos = ypos;
        (*Foods_List)->carrying = NULL;
        return *Foods_List;
    }

    struct food_struct* Food_Address = *Foods_List;
    while ((*Food_Address).next != NULL) { 
        Food_Address = (*Food_Address).next;
    }
    (*Food_Address).next =  (struct food_struct*)malloc(sizeof(struct food_struct)); //This is never freed
    (*(*Food_Address).next).next = NULL;
    (*(*Food_Address).next).xpos = xpos;
    (*(*Food_Address).next).ypos = ypos;
    (*(*Food_Address).next).carrying = NULL;
    (*(*Food_Address).next).last = Food_Address;
    return (*Food_Address).next;


}

struct colony_struct* init_colony(struct colony_struct** Colonys_List, double xpos, double ypos, double radius) {

    if(*Colonys_List == NULL) {
        *Colonys_List = (struct colony_struct*)malloc(sizeof(struct colony_struct)); 
        (*Colonys_List)->next = NULL;
        (*Colonys_List)->xpos = xpos;
        (*Colonys_List)->ypos = ypos;
        (*Colonys_List)->radius = radius;
        return *Colonys_List;
    }

    struct colony_struct* Colony_Address = *Colonys_List;
    while ((*Colony_Address).next != NULL) {
        Colony_Address = (*Colony_Address).next;
    }
    (*Colony_Address).next =  (struct colony_struct*)malloc(sizeof(struct colony_struct)); //This is never freed
    (*(*Colony_Address).next).next = NULL;
    (*(*Colony_Address).next).xpos = xpos;
    (*(*Colony_Address).next).ypos = ypos;
    (*(*Colony_Address).next).radius = radius;
    return (*Colony_Address).next;
}
struct ant_struct* init_ant(struct ant_struct** Ants_List, double xpos, double ypos) {
    //creates an ant and returns a poitner to it as well as adding it to the list

    if(*Ants_List == NULL) {
        *Ants_List = (struct ant_struct*)malloc(sizeof(struct ant_struct)); 
        (*Ants_List)->next = NULL;
        (*Ants_List)->state = Ant_Foraging;
        (*Ants_List)->angle = randfrom(0, 2*M_PI);
        (*Ants_List)->xpos = xpos;
        (*Ants_List)->ypos = ypos;
        return *Ants_List;
    }

    struct ant_struct* Ant_Address = *Ants_List;
    while ((*Ant_Address).next != NULL) {
        Ant_Address = (*Ant_Address).next;
    }
    (*Ant_Address).next =  (struct ant_struct*)malloc(sizeof(struct ant_struct)); //This is never freed
    (*(*Ant_Address).next).next = NULL;
    (*(*Ant_Address).next).state = Ant_Foraging;
    (*(*Ant_Address).next).angle = randfrom(0, 2*M_PI);
    (*(*Ant_Address).next).xpos = xpos;
    (*(*Ant_Address).next).ypos = ypos;
    return (*Ant_Address).next;
}

//ObjectInteractions:
int in_sense_range(struct ant_struct* Ant, void* Object, double* Target_X, double* Target_Y, double* Distance, int type) {

    if (type == Type_Food) {
        struct food_struct* Food = (struct food_struct*)Object;
        double Delta_X = Food->xpos - Ant->xpos;
        double Delta_Y = Food->ypos - Ant->ypos;
        double Magnitude_Squared = pow(Delta_X, 2) + pow(Delta_Y, 2);
        *Distance = Magnitude_Squared;
        
        *Target_X = Delta_X;
        *Target_Y = Delta_Y;
        
        if(Magnitude_Squared < pow(Ant_Sense_Range, 2)) {
            printf("Extra Sensory Preception Triggered\n");
            return 1;
        }
        return 0;
    }

    if(type == Type_Colony) {
        struct colony_struct* Colony = (struct colony_struct*)Object; 
        double Delta_X = Colony->xpos - Ant->xpos;
        double Delta_Y = Colony->ypos - Ant->ypos;
        double Magnitude_Squared = pow(Delta_X, 2) + pow(Delta_Y, 2);
        if (Magnitude_Squared < pow(Ant_Interact_Range + Colony->radius, 2)) {
            return 1;
        }
        return 0;
    }

    return -1;
}

int in_interact_range(struct ant_struct* Ant_Address, void* Object, int type) {
    
    if(type == Type_Food) {
        struct food_struct* Food = (struct food_struct*)Object;
        double Delta_X = (*Food).xpos - (*Ant_Address).xpos;
        double Delta_Y = (*Food).ypos - (*Ant_Address).ypos;
        double Magnitude_Squared = pow(Delta_X,2) + pow(Delta_Y, 2);
        if(Magnitude_Squared < pow(Ant_Interact_Range, 2)) {
            return 1;
        }
        return 0;
    }
    if(type == Type_Colony) {
       struct colony_struct* Colony = (struct colony_struct*)Object;
        double Delta_X = (*Colony).xpos - (*Ant_Address).xpos;
        double Delta_Y = (*Colony).ypos - (*Ant_Address).ypos;
        double Magnitude_Squared = pow(Delta_X,2) + pow(Delta_Y, 2);
        if(Magnitude_Squared < pow(Colony->radius,2)) {
            return 1;
        }
        return 0;  
    }
    return -1;
}

int pickup_food(struct ant_struct* Ant, struct food_struct* Food) {
    Ant->state = Ant_Homing;
    Ant->carrying = Food; 
    Food->carrying = Ant;
    return 0;
}

int in_view_cone(struct ant_struct* Ant_Address, void* Object, double* Target_X, double* Target_Y, double* Distance, int type) {

    if (type == Type_Food) {
        struct food_struct* Food = (struct food_struct*)Object;
        double Delta_X = (*Food).xpos - (*Ant_Address).xpos;
        double Delta_Y = (*Food).ypos - (*Ant_Address).ypos;
        double Relative_Angle = atan2(Delta_Y,Delta_X);
        double Delta_Angle = fabs(Relative_Angle-(*Ant_Address).angle); //Absoulte value
        double Magnitude_Squared = pow(Delta_X,2) + pow(Delta_Y, 2);
        *Distance = Magnitude_Squared;
        if((Magnitude_Squared <  pow(Ant_View_Range,2)) && (Delta_Angle < Ant_View_Angle)) {
            *Target_X = Delta_X;
            *Target_Y = Delta_Y;
            printf("Spotted some delicous food\n");
            return 1;

        }
        return 0;
    }
    if (type == Type_Trail) {
        return 0;
    }

    if (type == Type_Colony) {
        //TODO: Verify that this is actaully working
        //
        //To check if we can see this we need to determine if the view cone overlaps
        struct colony_struct* Colony = (struct colony_struct*)Object;
        double Delta_X = (*Colony).xpos - (*Ant_Address).xpos;
        double Delta_Y = (*Colony).ypos - (*Ant_Address).ypos;
        double Relative_Angle = atan2(Delta_Y,Delta_X);
        double Delta_Angle = fabs(Relative_Angle-(*Ant_Address).angle); //Absoulte value
        double Magnitude_Squared = pow(Delta_X,2) + pow(Delta_Y, 2);

        if(Magnitude_Squared <  pow(Ant_View_Range, 2) && Delta_Angle < Ant_View_Angle) {
            //update the poitner to the angle
            *Target_X = Delta_X;
            *Target_Y = Delta_Y;
            return 1;

        }

        //expand the colony bounding box into coefficents
        //y = cy +/- Sqrt(r^2 - x^2 +2 x cx - cx^2)

        //check if the left or right ray intersect the circle and lies on the reigion we care about

        //(y = m*x-m*ax+ay
        //m = tan(theta_ant +/- offset)
        //so putting it together gives us
        //m*x - m*ax + ay -cy = +/-Sqrt(r^2 - x^2 + 2*x*cx - cx^2)
        //Squaring to prouduce a quadratic
        //m^2*x^2 + 2*m*(ay-m*ax-cy)*x + (ay-m*ax-cy)^2 = -x^2 + 2*x*cx + (r^2-cx)
        //let s = ay-m*ax-cy
        //(m^2 + 1)*x^2 + (2*m*s-2*cx)*x +s^2+cx^2-r^2 = 0
        //Delta = (2*m*s-2*cx)^2 - 4*(m^2+1)*(s^2+cx^2-r^2) check the sign of this for intersection
        //then solve the x values, check if theyre in an acceptable range
        // x = 2*m*s 
        //xbound is ant_coord
        //xbound2 is Ant_View_Range*cos(oreintation p/m view angle) 

        //Positive Ray Check
        double m = tan((*Ant_Address).angle + Ant_View_Angle);
        double s = (*Ant_Address).ypos - m*(*Ant_Address).xpos - (*Colony).ypos;
        double Delta = pow(2*m*s-2*((*Colony).xpos), 2) - 4*(pow(m,2)+1)*(pow(s,2)+pow((*Colony).xpos,2)-pow((*Colony).radius,2));
        if (Delta > 0) {
            double xbound1 = (*Ant_Address).xpos;
            double xbound2 = (Ant_Address->xpos) + Ant_View_Range*cos((*Ant_Address).angle + Ant_View_Angle);
            double x1 = (-(2*m*s-2*(*Colony).xpos) + sqrt(Delta))/(2*(pow(m,2)+1));
            if (isValueInRange(x1, xbound1, xbound2)) {
                *Target_X = Delta_X;
                *Target_Y = Delta_Y;
                return 1;
            }
            double x2 = (-(2*m*s-2*(*Colony).xpos) - sqrt(Delta))/(2*(pow(m,2)+1));
            if (isValueInRange(x2, xbound1, xbound2)) {
                *Target_X = Delta_X;
                *Target_Y = Delta_Y;
                return 1;
            }
        }
        //Neagtive Ray Check
        m = tan((*Ant_Address).angle - Ant_View_Angle);
        s = (*Ant_Address).ypos - m*(*Ant_Address).xpos - (*Colony).ypos;
        Delta = pow(2*m*s-2*((*Colony).xpos), 2) - 4*(pow(m,2)+1)*(pow(s,2)+pow((*Colony).xpos,2)-pow((*Colony).radius,2));
        if (Delta > 0) {
            double xbound1 = (*Ant_Address).xpos;
            double xbound2 = (Ant_Address->xpos) + Ant_View_Range*cos((*Ant_Address).angle - Ant_View_Angle);

            double x1 = (-(2*m*s-2*(*Colony).xpos) + sqrt(Delta))/(2*(pow(m,2)+1));
            if (isValueInRange(x1, xbound1, xbound2)) {
                *Target_X = Delta_X;
                *Target_Y = Delta_Y;
                return 1;

            }
            double x2 = (-(2*m*s-2*(*Colony).xpos) - sqrt(Delta))/(2*(pow(m,2)+1));
            if (isValueInRange(x2, xbound1, xbound2)) {
                *Target_X = Delta_X;
                *Target_Y = Delta_Y;
                return 1;
            }
        }

        //Circle Arc Check
        //Colony Bounding Equation:
        //y = cy +/- Sqrt(r^2 - x^2 +2 x cx - cx^2)
        //Cone Arc Bounding Equation:
        //y = ay +/- Sqrt(AVR^2 - x^2 + 2 x ax - ax^2)
        //Combined

        //dx = cx-ax
        //dy = cy-ay
        //s = dx^2+dy^2

        //Delta = -dy^2*(r^4+(avr-s)^2-2r^2(avr+s)

        double dx = Colony->xpos - Ant_Address->xpos;
        double dy = Colony->xpos - Ant_Address->ypos;
        s = pow(dx,2) + pow(dy,2);
        Delta = -pow(dy,2)*( pow(Colony->radius,4) + (Ant_View_Range - s) - 2*pow( Colony->radius, 2)*(Ant_View_Range + s));
        if(Delta > 0) {
            double xbound1 = (Ant_Address->xpos) + Ant_View_Range*cos((*Ant_Address).angle - Ant_View_Angle);
            double xbound2 = (Ant_Address->xpos) + Ant_View_Range*cos((*Ant_Address).angle + Ant_View_Angle);

            double x1 = (Ant_View_Range*dx - dx*pow(Colony->radius, 2) + (2*Colony->xpos)*s + sqrt(Delta))/(2*s);
            if(isValueInRange(x1, xbound1,xbound2)) {
                *Target_X = Delta_X;
                *Target_Y = Delta_Y;               
                return 1;
            }
            double x2 = (Ant_View_Range*dx - dx*pow(Colony->radius, 2) + (2*Colony->xpos)*s - sqrt(Delta))/(2*s);;
            if(isValueInRange(x2, xbound1,xbound2)) {
                *Target_X = Delta_X;
                *Target_Y = Delta_Y;
                return 1;
            }
        }
        return 0;

    }
    return -1; //throw an error
}


void move_xy(struct ant_struct* Ant, double xRel, double yRel) {

    double Length_Squared = pow(xRel, 2) + pow(xRel, 2);
    double Correction_Factor = sqrt(Ant_Movement_Range/Length_Squared);
    Ant->xpos += xRel*Correction_Factor;
    Ant->ypos += yRel*Correction_Factor;
    Ant->angle = atan2(yRel, xRel);
}


void move_direction(struct ant_struct* Ant, double Move_Angle) {
    (*Ant).angle = Move_Angle;
    Ant->xpos = Ant->xpos +  (double)Ant_Movement_Range*cos(Move_Angle);
    Ant->ypos = Ant->ypos + (double)Ant_Movement_Range*sin(Move_Angle);
}

void move_randomly(struct ant_struct* Ant) {
   
    double Move_Angle = randfrom(-Ant_View_Angle, Ant_View_Angle);
    move_direction(Ant, Ant->angle + Move_Angle);
     
}

void delete_object(void* Object, int type){
    //this causes a segfault
    if(type == Type_Food) {
        struct food_struct* Food = (struct food_struct*)Object;
        (Food->next)->last = Food->last;
        (Food->last)->next = Food->next;
        free(Food);
    }
}

int render_ant(struct ant_struct* Ant, sf::RenderWindow* window) {

    //Render the View Cone
    sf::ConvexShape convex;
    convex.setPointCount(3);
    //must render all clockwise/ccw
    convex.setPoint(0, sf::Vector2f(Ant->xpos, Ant->ypos));

    float angle1 = Ant->angle - Ant_View_Angle;
    float angle2 = Ant->angle + Ant_View_Angle;

    if (angle1 > angle2) {
        std::swap(angle1, angle2);
    }

    convex.setPoint(1, sf::Vector2f(Ant->xpos + Ant_View_Range * cos(angle1), Ant->ypos + Ant_View_Range * sin(angle1)));
    convex.setPoint(2, sf::Vector2f(Ant->xpos + Ant_View_Range * cos(angle2), Ant->ypos + Ant_View_Range * sin(angle2)));



    convex.setFillColor(sf::Color(255, 0, 0));
    window->draw(convex);

    if (Ant->state == Ant_Foraging) {
        sf::CircleShape shape(3);
        shape.setFillColor(sf::Color(250, 0, 250));
        shape.setPosition(Ant->xpos, Ant->ypos);
        window->draw(shape);
        return 1;

    }
    if (Ant->state == Ant_Homing) {
        sf::CircleShape shape(3);
        shape.setFillColor(sf::Color(0, 0, 250));
        shape.setPosition(Ant->xpos, Ant->ypos);
        window->draw(shape);
        return 1;
    }
    return 0;
}

int render_food(struct food_struct* Food, sf::RenderWindow* window) {
    if (Food->carrying == NULL) {
        sf::CircleShape shape(3);
        shape.setFillColor(sf::Color(0, 255, 0));
        shape.setPosition(Food->xpos, Food->ypos);
        window->draw(shape);
        return 0;
    }
    sf::CircleShape shape(3);
    shape.setFillColor(sf::Color(0, 255, 0));
    shape.setPosition((Food->carrying)->xpos, (Food->carrying)->ypos);
    window->draw(shape);
    return 0;
}

int render_colony(struct colony_struct* Colony, sf::RenderWindow* window) {
    sf::CircleShape shape(Colony->radius);
    shape.setFillColor(sf::Color(255, 0, 0));
    shape.setPosition(Colony->xpos, Colony->ypos);
    window->draw(shape);
    return 0;
}

int ant_update(struct ant_struct* Ant_Address, struct food_struct* Foods_List, struct trail_struct* Foragings_List, struct trail_struct* Homings_List, struct colony_struct* Colonys_List) {

    //Ant needs to be dropping trails depending on its state

    if ((*Ant_Address).state == Ant_Foraging) {

        //check if food is left in the level
        if (Foods_List == NULL) {
            printf("No Food\n");
            return 0;
        } 

        struct food_struct* Food_Item = Foods_List;
        double Least_Mag_Squared = -1;
        double Move_X = 0;
        double Move_Y = 0;
        int Looping = 1;
        while(Looping) {

            //check if we can pick up the food
            if (in_interact_range(Ant_Address, Food_Item, Type_Food) && Food_Item->carrying == NULL) {
                pickup_food(Ant_Address, Food_Item);
                return 1;
            }
            double Food_Rel_X = 0;
            double Food_Rel_Y = 0;
            double Distance = 0;
            if ((in_view_cone(Ant_Address, Food_Item, &Food_Rel_X, &Food_Rel_Y, &Distance, Type_Food) || in_sense_range(Ant_Address, Food_Item, &Food_Rel_X, &Food_Rel_Y, &Distance, Type_Food)) && Food_Item->carrying == NULL) {
                if(Distance < Least_Mag_Squared || Least_Mag_Squared < 0) {
                    Least_Mag_Squared = Distance;
                    Move_X = Food_Rel_X;
                    Move_Y = Food_Rel_Y;
                }

            }
            if (Food_Item->next != NULL) {
                Food_Item = (Food_Item->next); // Follow the trail
            }
            else {
                Looping = 0; //Terminate the loop 
            }
        }

        if (Least_Mag_Squared > 0) {
            move_xy(Ant_Address, Move_X, Move_Y); 
            return 1;
        }




        //Can't see any food so we will check the trails
        if (Foragings_List == NULL) {
            move_randomly(Ant_Address);
            return 1;
        }

        //logic from food




    }

    if ((*Ant_Address).state == Ant_Homing) {
        //Check if you can see a colony
        struct colony_struct* Colony = Colonys_List;
        double Homing_Angle;
        double Move_Angle;
        double Max_Squared_Distance = -1;
        int Looping = 1;
        int Found = 0;
        while(Looping) {

            if(in_interact_range(Ant_Address, Colony, Type_Colony)) {

                delete_object(Ant_Address->carrying, Type_Food);
                Ant_Address->carrying = NULL; 
                Ant_Address->state = Ant_Foraging; 

                return 1;
            }

            //look for colonies

            if(Colony->next == NULL){
                Looping = 0;
            }
            else {
                Colony = Colony->next;
            }

        }

    }
    return -1;
}

struct setup_struct* setup() {
    struct setup_struct* Setup_Data = (struct setup_struct*)malloc(sizeof(struct setup_struct));
    Setup_Data->ants_list = NULL;
    for (int i = 0 ; i < Number_Ants; i++) {
        init_ant(&(Setup_Data->ants_list), randfrom(0,Canvas_X), randfrom(0, Canvas_Y));
    }
    Setup_Data->colonys_list = NULL;
    struct colony_struct* Colony = init_colony(&(Setup_Data->colonys_list), randfrom(0,Canvas_X), randfrom(0, Canvas_Y), 15.0);
    Setup_Data->foods_list = NULL;
    for (int i = 0 ; i < Number_Foods; i++) {
        struct food_struct* Food = init_food(&(Setup_Data->foods_list), randfrom(0,Canvas_X), randfrom(0, Canvas_Y));
        }
    Setup_Data->foragings_list = NULL;
    Setup_Data->homings_list = NULL;
    return Setup_Data;
}

void loop(sf::RenderWindow* window, struct ant_struct** Ants_List, struct trail_struct** Homings_List, struct trail_struct** Foragings_List, struct food_struct** Foods_List, struct colony_struct** Colonys_List) {

    
    // Set of Walls in the Level (Investigate what datatype to use)
    
    //ants
    struct ant_struct* Ant_Address = *Ants_List;
    while (Ant_Address->next != NULL) {
        ant_update(Ant_Address, *Foods_List, *Foragings_List, *Homings_List, *Colonys_List);
        render_ant(Ant_Address, window);
        //draw the ant
        Ant_Address = Ant_Address->next;
    }
    ant_update(Ant_Address, *Foods_List, *Foragings_List, *Homings_List, *Colonys_List);
    render_ant(Ant_Address, window);

    //foods
    struct food_struct* Food_Address = *Foods_List;
    while (Food_Address->next != NULL) {
        render_food(Food_Address, window);
        Food_Address = Food_Address->next;
    }
    render_food(Food_Address, window);
 
    struct colony_struct* Colony_Address = *Colonys_List;
    while (Colony_Address->next != NULL) {
        render_colony(Colony_Address, window);
        Colony_Address = Colony_Address->next;
    }
    render_colony(Colony_Address, window);


    /*
    struct trail_struct Homing_Address = *Homings_List;
    while (Homing_Address.next != NULL) {
        //trail_update(Homing_Address);
        Homing_Address = *Homing_Address.next;
    }
    //!!!Perform the loop contentx explicitly on the last element

    struct trail_struct Foraging_Address = *Foragings_List;
    while (Foraging_Address.next != NULL) {
        //trail_update(Foraging_Address);
        Foraging_Address = *Foraging_Address.next;
    }
    //!!!Perform the loop contentx explicitly on the last element

    //Loop that does something with Colonys

*/

    //Command to save the state

}


int main() {

    srand((unsigned int)time(NULL));
    struct setup_struct* Setup_Data = setup();
    
    //window init
    sf::RenderWindow window(sf::VideoMode(Canvas_X, Canvas_Y) , "Ant Simulation");
    window.setVerticalSyncEnabled(true); // call it once, after creating the window
    window.setActive(true);
    window.setFramerateLimit(60); //could have somethign like microstepping in this


    //Elaborate setupdata
    struct ant_struct* Ants_List = (*Setup_Data).ants_list;
    struct trail_struct* Homings_List = (*Setup_Data).homings_list;
    struct trail_struct* Foragings_List = (*Setup_Data).foragings_list;
    struct food_struct* Foods_List = (*Setup_Data).foods_list;
    struct colony_struct* Colonys_List = (*Setup_Data).colonys_list;

    //need to learn how to run things while the window is open
    int i = 0;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                //run the exit routine otherwise
                break;
            default:
                break;
            }

        }
        // clear the window with black color
        window.clear(sf::Color::Black);
//        printf("Looping %d\n", i);
        //loop function should deal with window 
        loop(&window, &Ants_List, &Homings_List, &Foragings_List, &Foods_List, &Colonys_List);
        i++; 
             
        // end the current frame
        window.display();



    }
    return 0;
}
