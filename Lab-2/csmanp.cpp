/*
Submitted by: Kushagra Kanodia
Waterloo ID: 20812200
File name: csmanp.cpp
Compilation command : g++ csmanp.cpp -o csmanp
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <queue>
#include <algorithm>
#include <stdlib.h>
#include <ctime>
using namespace std;
double T = 100;         //simulation time
double R = 1000000.0;   // speed of LAN
double L = 1500.0;      //Packet length
double D = 10.0;        //distance between adjacent nodes
double S = 200000000.0; //Propogation speed
struct Event
{
    string type;
    double time;
};
double ran_expo(double lambda) //function for generating random variables
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1 - u) / lambda;
}
struct Node //structure of every node
{
    vector<Event> q; //queue for each node
    int num_of_transmitted;
    int num_of_collision;      //counter for collision
    int num_of_collision_busy; // additional collision counter for wait time when channel is busy
    int num_of_successful;
    double T_arr;
    double T_waiting;
    bool sense;
};
Node n[100];                           // array of nodes
void generate_arrival(double A, int N) //generating arrival time of packets in nodes
{
    for (int i = 0; i < N; i++)
    {
        double t = 0.0;
        while (t < T)
        {
            Event e;
            t = t + ran_expo(A);
            e.time = t;
            n[i].q.push_back(e);

        }
    }
}
int randombackoff(int min, int max) //function for random backoff generation
{
    static bool first = true;
    if (first)
    {
        srand(time(NULL)); 
        first = false;
    }
    return min + rand() % ((max + 1) - min);
}
void processing(int N)
{

    int transmitter; //index of the transmitting node
    for (int j = 0; j < N; j++)
    {
        n[j].num_of_collision = 0;
        n[j].num_of_successful = 0;
        n[j].num_of_transmitted = 0;
    }

    for (int k = 0; k < N; k++)
    {
        while (!n[k].q.empty())
        {
            double min = 10000000.0;
            for (int i = 0; i < N; i++)
            {
                if (!n[i].q.empty())
                {
                    if (n[i].q[0].time < min) //selecting the transmitter node
                    {
                        min = n[i].q[0].time;
                        transmitter = i;
                    }
                }
            }
            for (int i = 0; i < N; i++)
            {
                if (i != transmitter)
                {
                    n[i].T_arr = ((abs(transmitter - i)) * (D / S)) + min; // first bit arrives at node i in time T_arr
                                                                          
                }
            }
            int flag = 0;
            for (int i = 0; i < N; i++)
            {

                if (!n[i].q.empty())
                {
                    if (n[i].T_arr > n[i].q[0].time) //Collision happens
                    {
                        flag = 1;
                        // cout << "Collision with" << i << "th node" << endl;
                        n[i].num_of_transmitted++; //Incrementing the transmission counter
                        n[i].num_of_collision++;  //Incrementing the collision counter
                        n[i].T_waiting = n[i].q[0].time + ((512 / R) * randombackoff(0, pow(2, n[i].num_of_collision - 1))); //Random backoff after collision
                        vector<Event>::iterator itr;
                        for (itr = n[i].q.begin(); itr != n[i].q.end(); itr++) //readjusting the arrival times after collision
                        {
                            if ((*itr).time < n[i].T_waiting)
                            {
                                (*itr).time = n[i].T_waiting;
                            }
                        }
                        if (n[i].num_of_collision > 10) // dropping the packet if collision counter exceeds 10
                        {
                            n[i].q.erase(n[i].q.begin());
                            n[i].num_of_collision = 0;
                        }
                    }
                }
            }
            if (flag == 1)
            {
                n[transmitter].num_of_collision++;
                n[transmitter].num_of_transmitted++;
                n[transmitter].T_waiting = n[transmitter].q[0].time + ((512 / R) * randombackoff(0, pow(2, n[transmitter].num_of_collision - 1)));
                vector<Event>::iterator itr1;
                for (itr1 = n[transmitter].q.begin(); itr1 != n[transmitter].q.end(); itr1++)
                {
                    if ((*itr1).time < n[transmitter].T_waiting)
                    {
                        (*itr1).time = n[transmitter].T_waiting;
                    }
                }
                if (n[transmitter].num_of_collision > 10)
                {
                    n[transmitter].q.erase(n[transmitter].q.begin());
                    n[transmitter].num_of_collision = 0;
                }
            }
            if (flag == 0) //Successful Transmission
            {
                n[transmitter].num_of_successful++; //Incrementing the successful counter
                // cout << "Sucessful transmission by " << transmitter << endl;
                n[transmitter].num_of_transmitted++;
                n[transmitter].num_of_collision = 0;  //Reset the collision counter
                n[transmitter].q.erase(n[transmitter].q.begin()); //pop the packet after successful
                vector<Event>::iterator itr2;
                for (int i = 0; i < N; i++)
                {
                    n[i].sense = false;
                    if (!n[i].q.empty())
                    {

                        for (itr2 = n[i].q.begin(); itr2 != n[i].q.end(); itr2++)
                        {
                            if ((*itr2).time > n[i].T_arr && (*itr2).time < (n[i].T_arr + (L / R)))//If channel busy
                            {
                                n[i].sense = true;
                                break;
                            }
                        }
                        if (n[i].sense == true)
                        {
                            n[i].num_of_collision_busy++;
                            vector<Event>::iterator it;
                            int randtime = randombackoff(0, pow(2, n[i].num_of_collision_busy - 1));//wait time when channel is sensed busy
                            for (it = n[i].q.begin(); it != n[i].q.end(); it++)
                            {
                                if ((*it).time > n[i].T_arr && (*it).time < (n[i].T_arr + (L / R)))
                                {
                                    (*it).time = (*it).time + ((512 / R) * randtime);
                                }
                            }
                        }
                        if (n[i].sense == false)//if channel sensed idle
                        {
                            n[i].num_of_collision_busy = 0; //reset the second collision counter
                        }
                    }
                }
            }
        }
    }
    int s1 = 0, s2 = 0;
    for (int i = 0; i < N; i++)
    {
        s1 = s1 + n[i].num_of_transmitted; //Adding all transmissions
        s2 = s2 + n[i].num_of_successful;  //Adding all successful transmissions
        //cout << n[i].num_of_successful << " successful out of " << n[i].num_of_transmitted << endl;
    }
    cout << s2 << " successful " << "out of " << s1 << endl;
    cout << "Efficiency for n = "<<N<<": " << s2 / (double)s1 << endl;//Printing the efficiency 
    cout<<"Throughput in mbps for n= "<<N<<": "<<s2*L/(double)(T*1000000)<<endl;
}
int main()
{

    for (int i = 20; i <= 100; i = i + 20)
    {
        generate_arrival(7.0, i); //Arrival rate=7
        processing(i);
    }
    for (int a = 20; a <= 100; a = a + 20)
    {
        generate_arrival(10.0, a); //Arrival rate=10
        processing(a);
    }
    for (int b = 20; b <= 100; b = b + 20)
    {
        generate_arrival(20.0, b); //Arrival rate=20
        processing(b);
    }
    return 0;
}