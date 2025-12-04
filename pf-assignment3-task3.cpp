#include <stdio.h>
#include <string.h>

struct employeeInfo {
	char name[50];
	int id;
	char designation[50];
	float salary;
};
void displayEmployees(struct employeeInfo emp[], int n){
	printf("ID\tName\t\tDesignation\tSalary\n");
	for (int i = 0; i < n; i++) {
    printf("%d\t%-10s\t%-12s\t%.2f\n", emp[i].id, emp[i].name, emp[i].designation, emp[i].salary);
    }
}
void findHighestSalary(struct employeeInfo emp[], int n){
	int high=0;
	for(int i=1 ; i<n ; i++)
	{
		if(emp[i].salary>emp[high].salary)
		high=i;
	}
	printf("\nEmployee with Highest Salary:\n");
    printf("ID: %d\n", emp[high].id);
    printf("Name: %s\n", emp[high].name);
    printf("Designation: %s\n", emp[high].designation);
    printf("Salary: %.2f\n", emp[high].salary);
}
void searchEmployee(struct employeeInfo emp[], int n){
	int choice;
    printf("\nSearch Employee:\n");
    printf("1. Search by ID\n");
    printf("2. Search by Name\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    
    if(choice==1)
    {
    	int searchID;
    	printf("Enter Employee ID: ");
    	scanf("%d",&searchID);
    	for(int i=0 ; i<n ; i++)
    	{
    		if(emp[i].id == searchID){
    		printf("ID\tName\t\tDesignation\tSalary\n");
    		printf("%d\t%-10s\t%-12s\t%.2f\n", emp[i].id, emp[i].name, emp[i].designation, emp[i].salary);
    		return ;
			}
		}
		printf("Employee not found\n");
	}
	else if(choice==2)
    {
    	char searchName[50];
    	printf("Enter Employee Name: ");
        scanf("%s", searchName);
    	for(int i=0 ; i<n ; i++)
    	{
    		if(strcmp(emp[i].name, searchName) == 0){
    		printf("ID\tName\t\tDesignation\tSalary\n");
    		printf("%d\t%-10s\t%-12s\t%.2f\n", emp[i].id, emp[i].name, emp[i].designation, emp[i].salary);
    		return ;
			}
		}
		printf("Employee not found\n");
	}
	else
	{
		printf("Invalid Choice");
	}
}

void updateSalary(struct employeeInfo emp[], int n, float threshold){
    printf("\n--- Updating Salaries (Bonus for Salary < %.2f) ---\n", threshold);
    int updated_count = 0;
    
    for(int i = 0; i < n; i++)
    {
        if(emp[i].salary < threshold)     // using the passed threshold
        {
            emp[i].salary *= 1.10;        // applying 10% bonus
            printf("Updated Employee %d (%s): New Salary = %.2f\n", 
                   emp[i].id, emp[i].name, emp[i].salary);
            updated_count++;
        }
    }
    if (updated_count == 0) {
        printf("No salaries were updated below the threshold.\n");
    }
}

int main()
{
	int n;
	printf("Enter number of employees:");
	if (scanf("%d",&n) != 1) return 1;
	
	while (getchar() != '\n'); 
	
	struct employeeInfo emp[n];
	
	for(int i=0; i<n ; i++)
	{
		printf("\nEnter Employee %d Details:\n", i + 1);
        printf("ID: ");
        if (scanf("%d", &emp[i].id) != 1) return 1;
        
        while (getchar() != '\n');
        
        printf("Name: ");
        fgets(emp[i].name, 50, stdin);
        emp[i].name[strcspn(emp[i].name, "\n")] = 0;
        
        printf("Designation: ");
        if (scanf("%s", emp[i].designation) != 1) return 1;
        
        printf("Salary: ");
        if (scanf("%f", &emp[i].salary) != 1) return 1;
        
        while (getchar() != '\n'); 
	}

	printf("\n\n------------- ALL EMPLOYEE RECORDS ----------------\n\n");
	displayEmployees(emp, n);

	printf("\n----------------------------------------------------------\n");
	findHighestSalary(emp, n);

	printf("\n----------------------------------------------------------\n");
	searchEmployee(emp, n);
    
    return 0; 
}


/* EXPLANATION: SALARY UPDATE BY REFERENCE

The function 'updateSalary(struct employeeInfo emp[], int n)' is designed to modify
the original employee data directly, which fulfills the requirement of "passing by
reference."

1.  How C Handles Arrays: In C, when we pass an array (like 'emp[]') to a function,
    the function doesn't get a copy of the entire array. Instead, it receives a
    pointer (a memory address) to the first element of the original array in the main function.

2.  Direct Modification: Because the function is working with the actual memory location
    of the employee data, any changes made inside the 'updateSalary' function (eg,
    'emp[i].salary *= 1.10;') are permanent.

3.  Implementation:
    a.  We iterate through the array from i=0 to n-1.
    b.  We check the condition (salary < 50000).
    c.  If the condition is met, we apply the 10% bonus using:
        'emp[i].salary = emp[i].salary + (emp[i].salary * 0.1);' (or simplified: 'emp[i].salary *= 1.10;')
    d.  Crucially, the function must NOT use 'return' until the loop is finished,
	    ensuring ALL employees under the threshold receive the bonus.
*/