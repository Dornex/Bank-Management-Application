#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "administrator.h"

typedef struct history{
	struct history *nextHistory;
}history;

typedef struct CARD{
	char nume[30];
	char expiry_date[10];
	char status[15];
	char pin[5];
	char pin_initial[5];
	char card_number[30];
	unsigned int balance;
	int inserted;
	int cvv;
	int strike;
	history *History;
	struct CARD *nextCard;
}CARD;

typedef struct LSC{
	unsigned int poz;
	unsigned int nr_max_carduri;
	struct LSC *nextLSC;
	CARD *nextCard;
}LSC;

void add_LSC(LSC **lsc, unsigned int poz)
{
 	LSC *elem = (LSC*)malloc(sizeof(LSC));
 	elem -> poz = poz;
 	elem -> nr_max_carduri = (*lsc) -> nr_max_carduri;
 	elem -> nextLSC = (*lsc);
 	(*lsc) = elem;
}

int verif_duplicate(LSC *lsc, unsigned int poz, char card_number[30])
{
	LSC *p = lsc;

	while(p!=NULL)
	{
		if(p->poz == poz) break;
		p = p->nextLSC;
	}

	if(p!=NULL)
	{
		CARD* card = p->nextCard;
		while(card!=NULL)
		{
			if(strcmp(card->card_number, card_number) == 0) return 1;
			card = card->nextCard;
		}
	}
	return 0;
}

int calculare_pozitie(LSC *lsc, char card_number[30])
{
	unsigned int suma = 0;

	int aux = atoi(card_number);
	while(aux)
	{
		suma += aux%10;
		aux /= 10;
	}
	return suma % (lsc->nr_max_carduri);
}

CARD* find_card(LSC *lsc, char card_number[30])
{
	unsigned int poz = calculare_pozitie(lsc, card_number);

	LSC *p = lsc;
	while(p!=NULL)
	{
		if(p->poz == poz) break;
		p = p->nextLSC;
	}

	CARD* c = p->nextCard;
	if(p!=NULL) 
	{		
		while(c!=NULL)
		{
			if(strcmp(c->card_number, card_number) == 0) break;
			c = c->nextCard;
		}
	}
	return c;
}

void add_card(char card_number[30], char pin[6], char expiry_date[10], int cvv, char nume[30], LSC **lsc)
{
	unsigned int poz = calculare_pozitie((*lsc), card_number);

	if(verif_duplicate((*lsc), poz, card_number) == 1) printf("The card already exists!\n");
	else
	{
		LSC *p = (*lsc);
		CARD *card = (CARD*)malloc(sizeof(CARD));

		while(p != NULL)
		{
			if(p->poz == poz) break;
			p = p->nextLSC;
		}

		strcpy(card->status, "NEW");
		strcpy(card -> card_number, card_number);
		strcpy(card -> pin, pin);
		strcpy(card -> pin_initial, pin);
		strcpy(card->expiry_date, expiry_date);
		strcpy(card->nume, nume);
		card -> strike = 0;
		card -> inserted = 0;
		card -> cvv = cvv;
		card -> balance = 0;
		
		if(p == NULL)
		{
			add_LSC(lsc, poz);
			(*lsc)->nextCard = card;
		}
		else
		{	
			card -> nextCard = p->nextCard;
			p -> nextCard = card;
		}
	} 
}

void afisare(LSC *lsc)
{
	LSC *p = lsc;
	while(p!=NULL)
	{
		CARD *c = p->nextCard;

		while(c!=NULL)
		{
			printf("%s %s %d %s \n", c->card_number, c->pin, c->cvv, c->expiry_date);
			c = c->nextCard;
		}
		p = p->nextLSC;
	}
}

void insert_card(LSC *lsc, char card_number[30], char pin[6])
{
	CARD *c = find_card(lsc, card_number);

	if(c->strike == 3) 
	{
		printf("The card is blocked. Please contact the administrator\n");
		strcpy(c->status, "BLOCKED");
	}
	else if(strcmp(c->pin, pin) != 0)
		{
			printf("Invalid PIN.\n");
			c->strike++;
		}
	else if(strcmp(c->pin, c->pin_initial) == 0)
	{		
		c->inserted = 1;			
		c->strike = 0;
		printf("You must change your PIN!\n");
	}
	else if(strcmp(c->pin, c->pin_initial) != 0) c->inserted = 1;
}

void cancel(LSC *lsc, char card_number[30])
{
	CARD *c = find_card(lsc, card_number);
	c -> inserted = 0;
}

void recharge(LSC *lsc, char card_number[30], unsigned int suma)
{
	CARD* c = find_card(lsc, card_number);
	if(c->inserted == 1)
	{
		if(suma % 10 != 0) printf("The added amount must be multiple of 10!\n");
		else 
		{
			
			c->balance += suma;
			printf("Sold curent: %d\n", c->balance);
		}
		cancel(lsc, card_number);
	}
	else printf("The card is not inserted!\n");	
}

void cash_withdrawal(LSC *lsc, char card_number[30], unsigned int suma)
{
	CARD* c = find_card(lsc, card_number);
	if(c->inserted == 1)
	{	
		if(suma > c->balance) printf("Insufficient funds!\n");
		else if(suma % 10 != 0) printf("The requested amount must be multiple of 10!\n");
		else
		{
			c->balance -= suma;
			printf("Sold ramas: %d\n", c->balance);
		}
		cancel(lsc, card_number);
	}
	else printf("The card is not inserted!\n");
}

void balance_inquiry(LSC *lsc, char card_number[30])
{
	CARD* c = find_card(lsc, card_number);
	if(c->inserted == 1)
	{	
		printf("Sold: %d\n", c->balance);
		cancel(lsc, card_number);
	}
	else printf("The card is not inserted!\n");
}

int main()
{
	FILE* fisier_in = fopen("intrare.txt", "r");
	FILE* fisier_out = fopen("iesire.txt", "w");

	unsigned int nr_max_carduri;
	fscanf(fisier_in, "%d", &nr_max_carduri);
	fprintf(fisier_out, "%d\n", nr_max_carduri);

	LSC *lsc = (LSC*)malloc(sizeof(LSC));
	lsc -> nr_max_carduri = nr_max_carduri;


	while(!feof(fisier_in))
	{

		char optiune[20], expiry_date[20];
		char card_number[30], pin[10];
		int cvv = 0; 

		fscanf(fisier_in, "%s", optiune);
		if(strcmp(optiune, "add_card") == 0)
		{	
			fscanf(fisier_in, "%s %s %s %d", card_number, pin, expiry_date, &cvv);
			add_card(card_number, pin, expiry_date, cvv, "Gigel", &lsc);	
			fflush(fisier_in);
		}
		else if(strcmp(optiune, "insert_card") == 0) 
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				insert_card(lsc, card_number, pin);
				fflush(fisier_in);
			}
			else if(strcmp(optiune, "recharge") == 0)
			{
				unsigned int suma;
				fscanf(fisier_in, "%s %d", card_number, &suma);
				recharge(lsc, card_number, suma);
			}
			else if(strcmp(optiune, "cash_withdrawal") == 0)
			{
				unsigned int suma;
				fscanf(fisier_in, "%s %d", card_number, &suma);
				cash_withdrawal(lsc, card_number, suma);
			}
			else if(strcmp(optiune, "balance_inquiry") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				balance_inquiry(lsc, card_number);
			}
		strcpy(optiune, "");
	}

	return 0;
}