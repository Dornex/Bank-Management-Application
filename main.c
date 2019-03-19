#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct history{
	char istoric[70]; 
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
 	LSC *elem = (LSC*)calloc(1, sizeof(LSC));
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
		CARD *card = (CARD*)calloc(1,sizeof(CARD));

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
			history *h = c->History;
			while(h!=NULL)
			{
				printf("{%s}, ", h->istoric);
				h = h->nextHistory;
			}
			c = c->nextCard;
		}
		p = p->nextLSC;
	}
}

void creare_history(char *hist, char rezultat[10], char operatie[15], char card_number_sursa[30], char card_number_dest[30], char pin[6], int sum)
{
	if(sum == -1) sprintf(hist, "%s %s %s %s%s", rezultat, operatie, card_number_sursa, card_number_dest, pin);
	else sprintf(hist, "%s %s %s %s%s%d", rezultat, operatie, card_number_sursa, card_number_dest, pin, sum);
}

void add_history(CARD **card, char History[70])
{
	history *elem = (history*)calloc(1, sizeof(history));
 	strcpy(elem->istoric, History);
 	elem->nextHistory = (*card)->History;
 	(*card)->History = elem;
}

void insert_card(LSC *lsc, char card_number[30], char pin[6])
{
	CARD *c = find_card(lsc, card_number);

	char hist[70];
	if(c->strike == 3)
	{
		printf("The card is blocked. Please contact the administrator\n");
		creare_history(hist, "FAIL", "insert_card", card_number, "", pin, -1);
		add_history(&c, hist);
	}
	else if(strcmp(c->pin, pin) != 0)
		{
			printf("Invalid PIN.\n");
			creare_history(hist, "FAIL", "insert_card", card_number, "", pin, -1);
			c->strike++;
			add_history(&c, hist);
		}
	else if(strcmp(c->pin, c->pin_initial) == 0)
		{
			c->inserted = 1;
			c->strike = 0;
			printf("You must change your PIN!\n");
			creare_history(hist, "SUCCES", "insert_card", card_number, "", pin, -1);
			add_history(&c, hist);
		}
	else if(strcmp(c->pin, c->pin_initial) != 0) 
		{
			creare_history(hist, "SUCCES", "insert_card", card_number, "", pin, -1);
			add_history(&c, hist);
		}
}	

void cancel(LSC *lsc, char card_number[30])
{
	CARD *c = find_card(lsc, card_number);

	char hist[70];
	creare_history(hist, "SUCCES", "cancel", card_number, "", "", -1);
	add_history(&c, hist);
}

void recharge(LSC *lsc, char card_number[30], unsigned int suma)
{
	CARD* c = find_card(lsc, card_number);
	char hist[70];

	if(suma % 10 != 0)
	{
		printf("The added amount must be multiple of 10!\n");
		creare_history(hist, "FAIL", "recharge", card_number, "", "", suma);
		add_history(&c, hist);	
	} 
	else
	{
		c->balance += suma;
		printf("Sold curent: %d\n", c->balance);
		creare_history(hist, "SUCCES", "recharge", card_number, "", "", suma);
		add_history(&c, hist);	
	}
	cancel(lsc, card_number);
}

void cash_withdrawal(LSC *lsc, char card_number[30], unsigned int suma)
{
	CARD* c = find_card(lsc, card_number);
	if(suma > c->balance) printf("Insufficient funds!\n");
	else if(suma % 10 != 0) printf("The requested amount must be multiple of 10!\n");
	else
	{
		c->balance -= suma;
		printf("Sold ramas: %d\n", c->balance);
	}
	cancel(lsc, card_number);
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

void pin_change(LSC *lsc, char card_number[30], char pin[6])
{
	if(strlen(pin)!=4) printf("Invalid PIN!\n");
	else
	{
		CARD *c = find_card(lsc, card_number);
		strcpy(c->pin, pin);
	}
	cancel(lsc, card_number);
}

void transfer_funds(LSC* lsc, char card_number_source[30], char card_number_dest[30], unsigned int suma)
{
	CARD* card_sursa = find_card(lsc, card_number_source);
	CARD* card_dest = find_card(lsc, card_number_dest);

	if(suma % 10 != 0) {printf("The trasnfered amount must be multiple of 10!\n"); cancel(lsc, card_number_source);}
	else if(suma > card_sursa->balance) {printf("Insufficient funds!\n"); cancel(lsc, card_number_source);}
	else
	{
		card_sursa->balance -= suma;
		card_dest->balance += suma;
		printf("Sold ramas: %d\n", card_sursa->balance);
		printf("Sold dest: %d\n", card_dest->balance);
		cancel(lsc, card_number_source);
	}
}

int main()
{
	FILE* fisier_in = fopen("intrare.txt", "r");
	FILE* fisier_out = fopen("iesire.txt", "w");

	unsigned int nr_max_carduri;
	fscanf(fisier_in, "%d", &nr_max_carduri);
	fprintf(fisier_out, "%d\n", nr_max_carduri);

	LSC *lsc = (LSC*)calloc(1, sizeof(LSC));
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
		}
		else if(strcmp(optiune, "insert_card") == 0)
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				insert_card(lsc, card_number, pin);
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
			else if(strcmp(optiune, "pin_change") == 0)
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				pin_change(lsc, card_number, pin);
			}
			else if(strcmp(optiune, "transfer_funds") == 0)
			{
				unsigned int suma;
				char card_number_source[30], card_number_dest[30];
				fscanf(fisier_in, "%s %s %d", card_number_source, card_number_dest, &suma);
				transfer_funds(lsc, card_number_source, card_number_dest, suma);
			}
			strcpy(optiune, "");
	}
	afisare(lsc);

	return 0;
}
