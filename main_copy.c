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
	char cvv[5];
	unsigned int balance;
	int inserted;
	int poz;
	int strike;
	history *History;
	struct CARD *nextCard;
}CARD;

typedef struct LSC{
	unsigned int poz;
	unsigned int afis_zero;
	unsigned int nr_max_carduri;
	unsigned int max_poz;
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

	char aux[30];
	strcpy(aux, card_number);
	int i;
	for(i = 0; i < strlen(aux); i++)
	{
		int x = aux[i] - '0';
		suma +=x;
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

void delete_card(LSC **lsc, char card_number[30])
{
	LSC *p = (*lsc);
	while(p!=NULL)
		if(p->nextCard != NULL)
		{
			CARD *c = p->nextCard;
			CARD *d = p->nextCard->nextCard;
			if(strcmp(c->card_number, card_number) == 0)
			{
				p->nextCard = d;
				free(c);
			}
			else
			{
				while(d!=NULL && strcmp(d->card_number, card_number) != 0)
				{
					c = c->nextCard;
					d = d->nextCard;
				}

				if(d!=NULL)
				{
					CARD*t = d;
					d = d->nextCard;
					c->nextCard = d;
					free(t);
				}
			}
		}
		p = p->nextLSC;
}

CARD *add_info_card(char card_number[30], char pin[6], char expiry_date[10], char cvv[5], char nume[30], int poz)
{
	CARD *card = (CARD *)calloc(1, sizeof(CARD));

	strcpy(card -> status, "NEW");
	strcpy(card -> card_number, card_number);
	strcpy(card -> pin, pin);
	strcpy(card -> pin_initial, pin);
	strcpy(card->expiry_date, expiry_date);
	strcpy(card->nume, nume);
	strcpy(card -> cvv, cvv);
	card -> strike = 0;
	card -> poz = poz;
	card -> inserted = 0;
	card -> balance = 0;
	return card;
}

void add_card(char card_number[30], char pin[6], char expiry_date[10], char cvv[5], char nume[30], LSC **lsc, FILE *fisier_out, int *poz_max, int *afis_zero)
{
	unsigned int poz = calculare_pozitie((*lsc), card_number);
	if(poz == 0) (*afis_zero) = 1;
	if(poz > (*poz_max)) (*poz_max) = poz;

	if(verif_duplicate((*lsc), poz, card_number) == 1) fprintf(fisier_out, "The card already exists\n");
	else
	{
		LSC *p = (*lsc);
		CARD *card = (CARD*)calloc(1,sizeof(CARD));

		while(p != NULL)
		{
			if(p->poz == poz) break;
			p = p->nextLSC;
		}

		card = add_info_card(card_number, pin, expiry_date, cvv, nume, poz);
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

void show_card(LSC *lsc, char card_number[30], FILE *fisier_out)
{
	CARD *card = find_card(lsc, card_number);
	fprintf(fisier_out, "(card number: %s, PIN: %s, expiry date: %s, CVV: %s, balance: %d, status: %s, history: [", card->card_number, card->pin, card->expiry_date, card->cvv, card->balance, card->status);
	history *h = card->History;
	while(h!=NULL)
	{
		if(h->nextHistory->istoric == NULL) fprintf(fisier_out, "%s", h->istoric);
		else fprintf(fisier_out, "%s, ", h->istoric);
		h = h->nextHistory;

	}
	fprintf(fisier_out, "])\n");
}

void show(LSC *lsc, FILE *fisier_out, int poz_max, int afis_zero)
{
	int i;
	for(i = 0; i <= poz_max; i++)
	{
		LSC *p = lsc;
		while(p!=NULL)
		{
			if(p->poz == i) break;
			p = p->nextLSC;
		}

		if(afis_zero == 0 && i == 0) {fprintf(fisier_out, "pos0: []\n"); continue;}

		if(p!=NULL)
		{
			fprintf(fisier_out, "pos%d: [\n", i);
			CARD *c = p->nextCard;
			while(c!=NULL)
			{
				fprintf(fisier_out, "(card number: %s, PIN: %s, expiry date: %s, CVV: %s, balance: %d, status: %s, history: [", c->card_number, c->pin, c->expiry_date, c->cvv, c->balance, c->status);
				history *h = c->History;
				while(h!=NULL)
				{
					if(h->nextHistory->istoric == NULL) fprintf(fisier_out, "%s", h->istoric);
					else fprintf(fisier_out, "%s, ", h->istoric);
					h = h->nextHistory;
				}
				fprintf(fisier_out, "])\n");
				c = c->nextCard;
			}
			fprintf(fisier_out, "]\n");
		}
		else fprintf(fisier_out, "pos%d: []\n", i);
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

void insert_card(LSC *lsc, char card_number[30], char pin[6], FILE *fisier_out)
{
	CARD *c = find_card(lsc, card_number);

	char hist[70];
	if(c->strike == 3)
	{
		fprintf(fisier_out, "The card is blocked. Please contact the administrator.\n");
		sprintf(hist, "%s %s %s %s%s", "(FAIL,", "insert_card", card_number, pin,")");
		strcmp(c->status, "BLOCKED");
		add_history(&c, hist);
	}
	else if(strcmp(c->pin, pin) != 0)
		{
			fprintf(fisier_out, "Invalid PIN.\n");
			sprintf(hist, "%s %s %s %s%s", "(FAIL,", "insert_card", card_number, pin,")");
			c->strike++;
			if(c->strike == 3) fprintf(fisier_out, "The card is blocked. Please contact the administrator.\n");
		}
	else if(strcmp(c->pin, c->pin_initial) == 0)
		{
			c->strike = 0;
			fprintf(fisier_out, "You must change your PIN.\n");
			sprintf(hist, "%s %s %s %s%s", "(SUCCESS,", "insert_card", card_number, pin,")");
		}
	else if(strcmp(c->pin, c->pin_initial) != 0)
			sprintf(hist, "%s %s %s %s%s", "(SUCCESS,", "insert_card", card_number, pin,")");
	add_history(&c, hist);
}

void cancel(LSC *lsc, char card_number[30], FILE *fisier_out)
{
	CARD *c = find_card(lsc, card_number);
	char hist[70];
	sprintf(hist, "%s %s %s%s", "(SUCCESS,", "cancel", card_number,")");
	add_history(&c, hist);
}

void recharge(LSC *lsc, char card_number[30], unsigned int suma, FILE *fisier_out)
{
	CARD* c = find_card(lsc, card_number);
	char hist[70];

	if(suma % 10 != 0)
	{
		fprintf(fisier_out, "The added amount must be multiple of 10!\n");
		sprintf(hist, "%s %s %s %d%s", "(FAIL,", "recharge", card_number, suma,")");
	}
	else
	{
		c->balance += suma;
		fprintf(fisier_out, "Sold curent: %d\n", c->balance);
		sprintf(hist, "%s %s %s %d%s", "(SUCCESS,", "recharge", card_number, suma,")");
	}
	add_history(&c, hist);
}

void cash_withdrawal(LSC *lsc, char card_number[30], unsigned int suma, FILE *fisier_out)
{
	CARD* c = find_card(lsc, card_number);
	char hist[70];

	if(suma > c->balance)
	{
		fprintf(fisier_out, "Insufficient funds!\n");
		sprintf(hist, "%s %s %s %d%s", "(FAIL,", "cash_withdrawal", card_number, suma,")");
	}
	else if(suma % 10 != 0)
	{
		fprintf(fisier_out, "The requested amount must be multiple of 10!\n");
		sprintf(hist, "%s %s %s %d%s", "(FAIL,", "cash_withdrawal", card_number, suma,")");
	}
	else
	{
		c->balance -= suma;
		fprintf(fisier_out, "Sold ramas: %d\n", c->balance);
		sprintf(hist, "%s %s %s %d%s", "(SUCCESS,", "cash_withdrawal", card_number, suma,")");
	}
	add_history(&c, hist);
}

void balance_inquiry(LSC *lsc, char card_number[30], FILE *fisier_out)
{
	CARD* c = find_card(lsc, card_number);
	char hist[70];

	fprintf(fisier_out, "Sold: %d\n", c->balance);
	sprintf(hist, "%s %s %s%s", "(SUCCESS,", "balance_inquiry", card_number,")");
	add_history(&c, hist);
}

void pin_change(LSC *lsc, char card_number[30], char pin[6], FILE *fisier_out)
{
	CARD *c = find_card(lsc, card_number);

	if(strlen(pin)!=4)
	{
		fprintf(fisier_out, "Invalid PIN!\n");
	}
	else
	{
		strcpy(c->pin, pin);
	}
}

void transfer_funds(LSC* lsc, char card_number_source[30], char card_number_dest[30], unsigned int suma, FILE *fisier_out)
{
	CARD* card_sursa = find_card(lsc, card_number_source);
	CARD* card_dest = find_card(lsc, card_number_dest);
	char hist[70];

	if(suma % 10 != 0)
	{
		fprintf(fisier_out, "The trasnfered amount must be multiple of 10!\n");
		sprintf(hist, "%s %s %s %s %d%s", "(FAIL,", "transfer_funds", card_number_source, card_number_dest, suma,")");
	}
	else if(suma > card_sursa->balance)
		{
			fprintf(fisier_out, "Insufficient funds!\n");
			sprintf(hist, "%s %s %s %s %d%s", "(FAIL,", "transfer_funds", card_number_source, card_number_dest, suma,")");
		}
		else
		{
			card_sursa->balance -= suma;
			card_dest->balance += suma;
			fprintf(fisier_out, "Sold ramas: %d\n", card_sursa->balance);
			fprintf(fisier_out, "Sold dest: %d\n", card_dest->balance);
			sprintf(hist, "%s %s %s %s %d%s", "(SUCCESS,", "transfer_funds", card_number_source, card_number_dest, suma,")");
		}
	add_history(&card_sursa, hist);
	add_history(&card_dest, hist);
}

void unblock_card(LSC *lsc, char card_number[30], FILE *fisier_out)
{
	CARD *card = find_card(lsc, card_number);
	strcmp(card->status, "ACTIVE");
	card->strike = 0;
}

int main()
{
	FILE* fisier_in = fopen("input.in", "r");
	FILE* fisier_out = fopen("output.out", "w");

	unsigned int nr_max_carduri;
	fscanf(fisier_in, "%d", &nr_max_carduri);

	LSC *lsc = (LSC*)calloc(1, sizeof(LSC));
	lsc -> poz = 0;
	lsc -> nr_max_carduri = nr_max_carduri;
	int poz_max = 0, afis_zero = 0;

	while(!feof(fisier_in))
	{
		char optiune[30], expiry_date[8];
		char card_number[30], pin[10], cvv[5];

		fscanf(fisier_in, "%s ", optiune);
		if(strcmp(optiune, "add_card") == 0)
		{
			fscanf(fisier_in, "%s %s %s %s", card_number, pin, expiry_date, cvv);
			add_card(card_number, pin, expiry_date, cvv, "Gigel", &lsc, fisier_out, &poz_max, &afis_zero);
		}
		else if(strcmp(optiune, "insert_card") == 0)
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				insert_card(lsc, card_number, pin, fisier_out);
			}
			else if(strcmp(optiune, "recharge") == 0)
			{
				unsigned int suma;
				fscanf(fisier_in, "%s %d", card_number, &suma);
				recharge(lsc, card_number, suma, fisier_out);
			}
			else if(strcmp(optiune, "cash_withdrawal") == 0)
			{
				unsigned int suma;
				fscanf(fisier_in, "%s %d", card_number, &suma);
				cash_withdrawal(lsc, card_number, suma, fisier_out);
			}
			else if(strcmp(optiune, "balance_inquiry") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				balance_inquiry(lsc, card_number, fisier_out);
			}
			else if(strcmp(optiune, "pin_change") == 0)
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				pin_change(lsc, card_number, pin, fisier_out);
			}
			else if(strcmp(optiune, "transfer_funds") == 0)
			{
				unsigned int suma;
				char card_number_source[30], card_number_dest[30];
				fscanf(fisier_in, "%s %s %d", card_number_source, card_number_dest, &suma);
				transfer_funds(lsc, card_number_source, card_number_dest, suma, fisier_out);
			}
			else if(strcmp(optiune, "show") == 0)
			{
				int nr = 0;
				strcpy(card_number, "-1");
				fseek(fisier_in, -strlen(optiune), SEEK_CUR);
				fgets(optiune, 30,  fisier_in);
				char *p = strtok(optiune, " ");

				while(p!=NULL)
				{
					nr++;
					if(nr == 1) strcpy(optiune, p);
					if(nr == 2) {strcpy(card_number, p); card_number[strlen(card_number)] = '\0';}
					p = strtok(NULL, " ");
				}

				if(strcmp(card_number, "-1") != 0) show_card(lsc, card_number, fisier_out);
				else show(lsc, fisier_out, poz_max, afis_zero);
			}
			else if(strcmp(optiune, "delete_card") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				delete_card(&lsc, card_number);
			}
			else if(strcmp(optiune, "cancel") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				cancel(lsc, card_number, fisier_out);
			}
			else if(strcmp(optiune, "unblock_card") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				unblock_card(lsc, card_number, fisier_out);
			}
			strcpy(optiune, "");
	}
	fclose(fisier_in);
	fclose(fisier_out);

	return 0;
}