#include <stdio.h>
#include <stdlib.h>

void calculateFuel(int fuel, int consumption, int recharge, int solarBonus, int planet, int totalPlanets)
{
	// base case 1
    if (planet>totalPlanets)
    {
        printf("\n MISSION COMPLETED! The spacecraft visited %d planets.\n", totalPlanets);
        return;
    }

    // consume fuel for the current leg
    fuel-=consumption;

    // apply gravitational recharge (every 3rd planet)
    if (planet%3== 0)
    {
        fuel+=recharge;
    }

    // apply solar recharge (every 4th planet)
    if (planet%4== 0)
    {
        fuel+=solarBonus;
    }
    
    // base case 2
    if (fuel<=0)
    {
        printf("Planet %d: Fuel Remaining = 0\n", planet);
        printf("\n MISSION FAILED! Fuel exhausted after Planet %d.\n", planet);
        return;
    }
    
    printf("Planet %d: Fuel Remaining = %d\n", planet, fuel);

    // recursion to the next planet
    calculateFuel(fuel, consumption, recharge, solarBonus, planet + 1, totalPlanets);
}

int main()
{
	int fuel, consumption, recharge, solarBonus, totalPlanets;
    
	printf("Enter fuel you will initiate with : ");
	if (scanf("%d", &fuel) != 1) return 1;
    
	printf("Enter fuel consumption per planet: ");
	if (scanf("%d", &consumption) != 1) return 1;
    
	printf("Enter gravitational recharge amount (every 3rd planet): ");
	if (scanf("%d", &recharge) != 1) return 1;
    
	printf("Enter solar Bonus amount (every 4th planet): ");
	if (scanf("%d", &solarBonus) != 1) return 1;
    
	totalPlanets = 12; 
    
	printf("\n------The Journey of Spacecraft begins (Total Planets: %d)------\n\n", totalPlanets);
    
	calculateFuel(fuel, consumption, recharge, solarBonus, 1, totalPlanets);

	return 0;
}