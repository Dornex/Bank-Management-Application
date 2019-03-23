#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
	int nr_carduri;
	struct LSC *nextLSC;
	CARD *nextCard;
}LSC;

typedef struct LC{
	LSC *infoLSC;
	int nr_max_carduri;
}LC;

void add_LSC(LC **lc, unsigned int poz)
{
 	LSC *elem = (LSC*)calloc(1, sizeof(LSC));
 	elem -> nr_carduri = 0;
 	elem -> poz = poz;
 	elem -> nr_max_carduri = (*lc) -> nr_max_carduri;
 	elem -> nextLSC = (*lc) -> infoLSC;
 	(*lc) -> infoLSC = elem;
}

int verif_duplicate(LC *lc, unsigned int poz, char card_number[30])
{
	LSC *p = lc -> infoLSC;

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

int calculare_pozitie(LC *lc, char card_number[30])
{
	unsigned int suma = 0;

	char aux[30];
	strcpy(aux, card_number);
	int i;
	for(i = 0; i < strlen(aux); i++)
	{
		int x = aux[i] - '0';
		suma += x;
	}
	return suma % (lc -> nr_max_carduri);
}

CARD* find_card(LC *lc, char card_number[30])
{
	unsigned int poz = calculare_pozitie(lc, card_number);

	LSC *p = lc -> infoLSC;

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

void delete_card(LC **lc, char card_number[30], int *afis_zero)
{
	LSC *p = (*lc)->infoLSC;
	while(p!=NULL)
	{
		if(p->nextCard != NULL)
		{
			CARD *c = p->nextCard;
			CARD *d = p->nextCard->nextCard;
			if(strcmp(c->card_number, card_number) == 0)
			{
				p->nextCard = d;
				free(c);
				p->nr_carduri--;			
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
					p->nr_carduri --;
					free(t);
				}
			}
		}
		p = p->nextLSC;
	}
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

void add_card(char card_number[30], char pin[6], char expiry_date[10], char cvv[5], char nume[30], LC **lc, FILE *fisier_out, int *poz_max, int *afis_zero)
{
	unsigned int poz = calculare_pozitie((*lc), card_number);
	if(poz > (*poz_max)) (*poz_max) = poz;

	if(verif_duplicate((*lc), poz, card_number) == 1) fprintf(fisier_out, "The card already exists\n");
	else
	{
		LSC *p = (*lc) -> infoLSC;
		CARD *card = (CARD*)calloc(1,sizeof(CARD));

		while(p != NULL)
		{
			if(p->poz == poz) break;
			p = p->nextLSC;
		}

		card = add_info_card(card_number, pin, expiry_date, cvv, nume, poz);
		if(p == NULL)
		{
			add_LSC(lc, poz);
			(*lc)->infoLSC->nextCard = card;
			(*lc)->infoLSC->nr_carduri = (*lc)->infoLSC->nr_carduri + 1;
		}
		else
		{
			card -> nextCard = p -> nextCard;
			p -> nextCard = card;
			p -> nr_carduri = p -> nr_carduri + 1;
		}
	}
}

void show_card(LC *lc, char card_number[30], FILE *fisier_out)
{
	CARD *card = find_card(lc, card_number);
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

void show(LC *lc, FILE *fisier_out, int poz_max, int afis_zero)
{
	int i;
	for(i = 0; i <= poz_max; i++)
	{
		LSC *p = lc->infoLSC;
		while(p!=NULL)
		{
			if(p->poz == i) break;
			p = p->nextLSC;
		}

		if(p!=NULL)
			{if(p->nr_carduri == 0) fprintf(fisier_out, "pos%d: []\n",i);
				else if(p!=NULL)
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
 	if((*card)->History == NULL) 
 	{
 		elem->nextHistory = NULL;
 		(*card)->History = elem;

 	}
 	else 
 	{
 		elem -> nextHistory = (*card)->History;
 		(*card)->History = elem;
 	}
}

void insert_card(LC *lc, char card_number[30], char pin[6], FILE *fisier_out)
{
	CARD *c = find_card(lc, card_number);

	char hist[70];
	if(strcmp(c->pin, pin) != 0)
	{
		if(c->strike == 3) {strcpy(c->status, "LOCKED");fprintf(fisier_out, "The card is blocked. Please contact the administrator.\n");}
		else
		{
			c->strike++;
			fprintf(fisier_out, "Invalid PIN\n");
			sprintf(hist, "%s %s %s %s%s", "(FAIL,", "insert_card", card_number, pin,")");
			if(c->strike == 3) {strcpy(c->status, "LOCKED");fprintf(fisier_out, "The card is blocked. Please contact the administrator.\n");}
		}	
	}
	else if(strcmp(c->pin, c->pin_initial) == 0 )
		{
			if(c->strike == 3) 
			{
				strcpy(c->status, "LOCKED");
				fprintf(fisier_out, "The card is blocked. Please contact the administrator.\n");
				sprintf(hist, "%s %s %s %s%s", "(FAIL,", "insert_card", card_number, pin,")");
			}
			else
			{
				c->strike = 0;
				fprintf(fisier_out, "You must change your PIN.\n");
				sprintf(hist, "%s %s %s %s%s", "(SUCCESS,", "insert_card", card_number, pin,")");
			}
			
		}
	else if(strcmp(c->pin, c->pin_initial) != 0)
			sprintf(hist, "%s %s %s %s%s", "(SUCCESS,", "insert_card", card_number, pin,")");
	add_history(&c, hist);
}

void cancel(LC *lc, char card_number[30], FILE *fisier_out)
{
	CARD *c = find_card(lc, card_number);
	char hist[70];
	sprintf(hist, "%s %s %s%s", "(SUCCESS,", "cancel", card_number,")");
	add_history(&c, hist);
}

void recharge(LC *lc, char card_number[30], unsigned int suma, FILE *fisier_out)
{
	CARD* c = find_card(lc, card_number);
	char hist[70];

	if(suma % 10 != 0)
	{
		fprintf(fisier_out, "The added amount must be multiple of 10\n");
		sprintf(hist, "%s %s %s %d%s", "(FAIL,", "recharge", card_number, suma,")");
	}
	else
	{
		c->balance += suma;
		fprintf(fisier_out, "%d\n", c->balance);
		sprintf(hist, "%s %s %s %d%s", "(SUCCESS,", "recharge", card_number, suma,")");
	}
	add_history(&c, hist);
}

void cash_withdrawal(LC *lc, char card_number[30], unsigned int suma, FILE *fisier_out)
{
	CARD* c = find_card(lc, card_number);
	char hist[70];

	if(suma % 10 != 0)
	{
		fprintf(fisier_out, "The requested amount must be multiple of 10\n");
		sprintf(hist, "%s %s %s %d%s", "(FAIL,", "cash_withdrawal", card_number, suma,")"); 
	}
	else if(suma > c->balance)
	{
		fprintf(fisier_out, "Insufficient funds\n");
		sprintf(hist, "%s %s %s %d%s", "(FAIL,", "cash_withdrawal", card_number, suma,")");
	}
	else
	{
		c->balance -= suma;
		fprintf(fisier_out, "%d\n", c->balance);
		sprintf(hist, "%s %s %s %d%s", "(SUCCESS,", "cash_withdrawal", card_number, suma,")");
	}
	add_history(&c, hist);
}

void balance_inquiry(LC *lc, char card_number[30], FILE *fisier_out)
{
	CARD* c = find_card(lc, card_number);
	char hist[70];

	fprintf(fisier_out, "%d\n", c->balance);
	sprintf(hist, "%s %s %s%s", "(SUCCESS,", "balance_inquiry", card_number,")");
	add_history(&c, hist);
}

void pin_change(LC *lc, char card_number[30], char pin[6], FILE *fisier_out)
{
	CARD *c = find_card(lc, card_number);
	char hist[70];
	int ok = 0;
	for(int i = 0; i < strlen(pin); i++)
	{
		if(!isdigit(pin[i]))
		{
			ok = 1;
			fprintf(fisier_out, "Invalid PIN\n");
			sprintf(hist, "%s %s %s %s%s", "(FAIL,", "pin_change", card_number, pin, ")");
			break;
		}
	}

	if(ok == 0)
	{
		if(strlen(pin)!=4)
		{
			sprintf(hist, "%s %s %s %s%s", "(FAIL,", "pin_change", card_number, pin, ")");
			fprintf(fisier_out, "Invalid PIN\n");
		}
		else
		{
			sprintf(hist, "%s %s %s %s%s", "(SUCCESS,", "pin_change", card_number, pin, ")");
			strcpy(c->pin, pin);
			strcpy(c->status, "ACTIVE");
		}
	}
	add_history(&c, hist);
}

void transfer_funds(LC* lc, char card_number_source[30], char card_number_dest[30], unsigned int suma, FILE *fisier_out)
{
	CARD* card_sursa = find_card(lc, card_number_source);
	CARD* card_dest = find_card(lc, card_number_dest);
	char hist[70];

	if(suma % 10 != 0)
	{
		fprintf(fisier_out, "The transferred amount must be multiple of 10\n");
		sprintf(hist, "%s %s %s %s %d%s", "(FAIL,", "transfer_funds", card_number_source, card_number_dest, suma,")");
		add_history(&card_sursa, hist);
	}
	else if(suma > card_sursa->balance)
		{
			fprintf(fisier_out, "Insufficient funds\n");
			sprintf(hist, "%s %s %s %s %d%s", "(FAIL,", "transfer_funds", card_number_source, card_number_dest, suma,")");
			add_history(&card_sursa, hist);
		}
		else
		{
			card_sursa->balance -= suma;
			card_dest->balance += suma;
			fprintf(fisier_out, "%d\n", card_sursa->balance);
			sprintf(hist, "%s %s %s %s %d%s", "(SUCCESS,", "transfer_funds", card_number_source, card_number_dest, suma,")");
			add_history(&card_sursa, hist);
			add_history(&card_dest, hist);
		}	
}

void remove_history(CARD **card, char hist[70])
{
	history *p = (*card)->History;
	if(strcmp(p->istoric, hist) == 0)
	{
		history *q = p;
		p = p -> nextHistory;
		free(q);
	}
	else
	{
		history *q = p;
		while (strcmp(q->nextHistory->istoric, hist) != 0 && q) q = q->nextHistory;
		history *a = q->nextHistory;
		q->nextHistory = a->nextHistory;
		free(a);
	}
}

void reverse_transaction(LC *lc, char card_number_source[30], char card_number_dest[30], unsigned int suma, FILE *fisier_out)
{
	CARD *card_source = find_card(lc, card_number_source);
	CARD *card_dest = find_card(lc, card_number_dest);
	char hist[70], hist2[70];

	if(card_dest->balance < suma) fprintf(fisier_out, "The transaction cannot be reversed\n");
	else
	{
		card_dest->balance -= suma;
		card_source->balance += suma;

		sprintf(hist, "%s %s %s %s %d%s", "(SUCCESS,", "reverse_transaction", card_number_source, card_number_dest, suma,")");
		sprintf(hist2, "%s %s %s %s %d%s", "(SUCCESS,", "transfer_funds", card_number_source, card_number_dest, suma, ")");

		remove_history(&card_dest, hist2);
		add_history(&card_source, hist);
	}
}

void unblock_card(LC *lc, char card_number[30], FILE *fisier_out)
{
	CARD *card = find_card(lc, card_number);
	strcpy(card->status, "ACTIVE");
	card->strike = 0;
}

int main()
{
	FILE* fisier_in = fopen("input.in", "r");
	FILE* fisier_out = fopen("output.out", "w");

	unsigned int nr_max_carduri;
	fscanf(fisier_in, "%d", &nr_max_carduri);

	LC *lc = (LC*)calloc(1, sizeof(LC));
	LSC *lsc = (LSC*)calloc(1, sizeof(LSC));
	lsc->poz = 0;

	lc->nr_max_carduri = nr_max_carduri;
	lc->infoLSC = lsc;

	int poz_max = 0, afis_zero = 0;

	while(!feof(fisier_in))
	{
		char optiune[30], expiry_date[8];
		char card_number[30], pin[10], cvv[5];

		fscanf(fisier_in, "%s ", optiune);
		if(strcmp(optiune, "add_card") == 0)
		{
			fscanf(fisier_in, "%s %s %s %s", card_number, pin, expiry_date, cvv);
			add_card(card_number, pin, expiry_date, cvv, "Gigel", &lc, fisier_out, &poz_max, &afis_zero);
		}
		else if(strcmp(optiune, "insert_card") == 0)
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				insert_card(lc, card_number, pin, fisier_out);
			}
			else if(strcmp(optiune, "recharge") == 0)
			{
				unsigned int suma;
				fscanf(fisier_in, "%s %d", card_number, &suma);
				recharge(lc, card_number, suma, fisier_out);
			}
			else if(strcmp(optiune, "cash_withdrawal") == 0)
			{
				unsigned int suma;
				fscanf(fisier_in, "%s %d", card_number, &suma);
				cash_withdrawal(lc, card_number, suma, fisier_out);
			}
			else if(strcmp(optiune, "balance_inquiry") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				balance_inquiry(lc, card_number, fisier_out);
			}
			else if(strcmp(optiune, "pin_change") == 0)
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				pin_change(lc, card_number, pin, fisier_out);
			}
			else if(strcmp(optiune, "transfer_funds") == 0)
			{
				unsigned int suma;
				char card_number_source[30], card_number_dest[30];
				fscanf(fisier_in, "%s %s %d", card_number_source, card_number_dest, &suma);
				transfer_funds(lc, card_number_source, card_number_dest, suma, fisier_out);
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
					if(nr == 2) {strcpy(card_number, p); card_number[strlen(card_number)-1] = '\0';}
					p = strtok(NULL, " ");
				}

				if(strcmp(card_number, "-1") != 0) show_card(lc, card_number, fisier_out);
				else show(lc, fisier_out, poz_max, afis_zero);
			}
			else if(strcmp(optiune, "delete_card") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				delete_card(&lc, card_number, &afis_zero);
			}
			else if(strcmp(optiune, "cancel") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				cancel(lc, card_number, fisier_out);
			}
			else if(strcmp(optiune, "unblock_card") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				unblock_card(lc, card_number, fisier_out);
			}
			else if(strcmp(optiune, "reverse_transaction") == 0)
			{
				int suma;
				char card_number_source[30], card_number_dest[30];
				fscanf(fisier_in, "%s %s %d", card_number_source, card_number_dest, &suma);
				printf("%s %s %s %d\n", optiune, card_number_source, card_number_dest, suma);
				reverse_transaction(lc, card_number_source, card_number_dest, suma, fisier_out);
			}
			strcpy(optiune, "");
	}
	fclose(fisier_in);
	fclose(fisier_out);

	return 0;
}
