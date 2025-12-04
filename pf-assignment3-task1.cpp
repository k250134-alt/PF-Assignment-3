#include <stdio.h>

double calculateRepayment(double loan, double interestRate, int totalYears, int currentYear) {

    const double installment = 20000;
    
    if (currentYear > totalYears) {
        return 0;
    }
    
    //  applying interest to the previous years balance
    double interestPaid = loan * interestRate;
    loan += interestPaid;
    
    //  pay the fixed installment
    loan -= installment;
    
    //  handling overpayment
    if (loan < 0) {
        loan = 0;
        //  in this case, the total repayment will still include the full installment amount
    }
    
    //  printing the remaining loan for the current year
    printf("Year %d: Remaining loan = %.2f\n", currentYear, loan);
    
    //  recursion: total repayment = current installment + repayment for remaining years
    return installment + calculateRepayment(loan, interestRate, totalYears, currentYear + 1);
}

int main() {

	double loan, interestRate;
	int years;

	printf("Enter loan amount: ");
    if (scanf("%lf", &loan) != 1) return 1;
    
    printf("Enter Interest Rate(percent): ");
    if (scanf("%lf", &interestRate) != 1) return 1;
    
    interestRate = interestRate / 100.0;
    
    printf("Enter Years: ");
    if (scanf("%d", &years) != 1) return 1;

	printf("\n---------- LOAN SCHEDULE -----------\n");
	
	double total = calculateRepayment(loan, interestRate, years, 1);

	printf("\nTotal repayment over %d years = %.2f\n", years, total);

	return 0;
}


/* EXPLANATION: RECURSIVE LOAN CALCULATION

This recursive function calculates the loan schedule by showing how the loan balance
changes step-by-step over the years.

1.  How Recursion Tracks State (Loan Balance):
    Recursion is excellent approach here because the starting balance for any "Year N" is directly
    dependent on the final balance from "Year N-1". Each function call is a single year.
    The 'loan' variable acts as the state that is passed and updated across these yearly steps.
    

2.  Suitability for Incremental Calculations:
    Recursion is suitable because the calculation is incremental which relies on the result
    of the previous step. The function stops (Base Case) only when the loan state (balance)
    reaches zero or the time state (years) reaches zero.

3.  Logical Extension for Extra Yearly Payments:
    To handle an extra yearly payment, we would modify the function to accept a new parameter,
    like 'extraPaymentAmount'. Inside the recursive step, we would simply deduct this extra payment
    from the 'loan' balance after the fixed installment. 

	The extended function would look like this:

	calculateRepayment(loan, interestRate, totalYears, currentYear, extraPaymentAmount))
    return installment + calculateRepayment(loan - extraPaymentAmount, interestRate, totalYears, currentYear + 1, extraPaymentAmount);
*/
