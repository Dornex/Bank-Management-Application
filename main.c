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
	int poz;
	int poz_veche;
	int strike;
	history *History;
	struct CARD *nextCard;
}CARD;

typedef struct LSC{
	CARD *nextCard;
}LSC;

typedef struct LC{
	LSC *infoLSC;
	int nr_max_carduri;
	int nr_carduri;
	int poz;
	int nr_carduri_totale;
	struct LC *nextLC;
}LC;

void add_LC(LC **lc, unsigned int poz)
{
 	LC *elem = (LC*)calloc(1, sizeof(LC));
 	LSC *lsc = (LSC*)calloc(1, sizeof(LSC));

 	elem -> nr_carduri = 0;
 	elem -> poz = poz;
 	elem -> infoLSC = lsc;
 	elem -> nr_max_carduri = (*lc) -> nr_max_carduri;
 	elem -> nextLC = (*lc);
 	(*lc) = elem;
}

int verif_duplicate(LC *lc, unsigned int poz, char card_number[30])
{
	LC *p = lc;

	while(p!=NULL)
	{
		if(p->poz == poz) break;
		p = p->nextLC;
	}
	if(p!=NULL)
	{
		CARD* card = p->infoLSC->nextCard;
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

CARD* find_card(LC **lc, char card_number[30])
{
	unsigned int poz = calculare_pozitie((*lc), card_number);
	LC *p = (*lc);

	while(p!=NULL)
	{
		if(p->poz == poz) break;
		p = p->nextLC;
	}
	CARD* c = p->infoLSC->nextCard;
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

void delete_card(LC **lc, char card_number[30])
{
	LC *p = (*lc);
	while(p!=NULL)
	{
		if(p->infoLSC->nextCard != NULL)
		{
			CARD *c = p->infoLSC->nextCard;
			CARD *d = p->infoLSC->nextCard->nextCard;
			if(strcmp(c->card_number, card_number) == 0)
			{
				p->infoLSC->nextCard = d;
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
					CARD *t = d;
					d = d->nextCard;
					c->nextCard = d;
					p->nr_carduri --;
					free(t);
				}
			}
		}
		p = p->nextLC;
	}
}

CARD *add_info_card(char card_number[30], char pin[6], char expiry_date[10], char cvv[5], int poz)
{
	CARD *card = (CARD *)calloc(1, sizeof(CARD));
	strcpy(card -> status, "NEW");
	strcpy(card -> card_number, card_number);
	strcpy(card -> pin, pin);
	strcpy(card -> pin_initial, pin);
	strcpy(card -> expiry_date, expiry_date);
	strcpy(card -> cvv, cvv);
	card -> History = NULL;
	card -> strike = 0;
	card -> poz = poz;
	card -> balance = 0;
	return card;
}

void add_card(char card_number[30], char pin[6], char expiry_date[10], char cvv[5], LC **lc, FILE *fisier_out, int *poz_max)
{
	unsigned int poz = calculare_pozitie((*lc), card_number);
	if(poz > (*poz_max)) (*poz_max) = poz;

	if(verif_duplicate((*lc), poz, card_number) == 1) fprintf(fisier_out, "The card already exists\n");
	else
	{
		LC *p = (*lc);
		CARD *card = (CARD*)calloc(1,sizeof(CARD));

		while(p != NULL)
		{
			if(p->poz == poz) break;
			p = p->nextLC;
		}

		card = add_info_card(card_number, pin, expiry_date, cvv, poz);
		if(p == NULL)
		{
			add_LC(lc, poz);
			(*lc)->infoLSC->nextCard = card;
			(*lc)->nr_carduri = (*lc)->nr_carduri + 1;
		}
		else
		{
			card -> nextCard = p -> infoLSC -> nextCard;
			p -> infoLSC -> nextCard = card;
			p -> nr_carduri = p -> nr_carduri + 1;
		}
	}
}

void show_card(LC *lc, char card_number[30], FILE *fisier_out)
{
	CARD *card = find_card(&lc, card_number);
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

void show(LC *lc, FILE *fisier_out, int poz_max)
{
	int i;
	for(i = 0; i <= poz_max; i++)
	{
		LC *p = lc;
		while(p!=NULL)
		{
			if(p->poz == i) break;
			p = p->nextLC;
		}
		if(p!=NULL)
			{
				if(p->nr_carduri == 0) fprintf(fisier_out, "pos%d: []\n",i);
				else if(p!=NULL)
				{
					fprintf(fisier_out, "pos%d: [\n", i);
					CARD *c = p->infoLSC->nextCard;
					while(c!=NULL)
					{
						fprintf(fisier_out, "(card number: %s, PIN: %s, expiry date: %s, CVV: %s, balance: %d, status: %s, history: [", c->card_number, c->pin, c->expiry_date, c->cvv, c->balance, c->status);
						history *h = c->History;
						while(h!=NULL)
                        {
                            if(strcmp(h->istoric, "-1") != 0)
                            {
                                if(h->nextHistory->istoric == NULL) fprintf(fisier_out, "%s", h->istoric);
                                else 
                                {	
                                	if(h->nextHistory->nextHistory->istoric == NULL && strcmp(h->nextHistory->istoric, "-1") == 0) fprintf(fisier_out, "%s", h->istoric);
                                	else fprintf(fisier_out, "%s, ", h->istoric);
                                }
                            }
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

void insert_card(LC **lc, char card_number[30], char pin[6], FILE *fisier_out)
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
				if(strcmp(c->status, "ACTIVE") != 0) fprintf(fisier_out, "You must change your PIN.\n");
				sprintf(hist, "%s %s %s %s%s", "(SUCCESS,", "insert_card", card_number, pin,")");
			}
		}
	else if(strcmp(c->pin, c->pin_initial) != 0)
			{
				if(c->strike == 3)
				{
					fprintf(fisier_out, "The card is blocked. Please contact the administrator.\n");
					sprintf(hist, "%s %s %s %s%s", "(FAIL,", "insert_card", card_number, pin,")");
				}
				else sprintf(hist, "%s %s %s %s%s", "(SUCCESS,", "insert_card", card_number, pin,")");
			}
	add_history(&c, hist);
}

void cancel(LC *lc, char card_number[30], FILE *fisier_out)
{
	CARD *c = find_card(&lc, card_number);
	char hist[70];
	sprintf(hist, "%s %s %s%s", "(SUCCESS,", "cancel", card_number,")");
	add_history(&c, hist);
}

void recharge(LC *lc, char card_number[30], unsigned int suma, FILE *fisier_out)
{
	CARD* c = find_card(&lc, card_number);
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
	CARD* c = find_card(&lc, card_number);
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
	CARD* c = find_card(&lc, card_number);
	char hist[70];
	fprintf(fisier_out, "%d\n", c->balance);
	sprintf(hist, "%s %s %s%s", "(SUCCESS,", "balance_inquiry", card_number,")");
	add_history(&c, hist);
}

void pin_change(LC *lc, char card_number[30], char pin[6], FILE *fisier_out)
{
	CARD *c = find_card(&lc, card_number);
	char hist[70];
	int ok = 0;
	int i;
	for(i = 0; i < strlen(pin); i++)
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
	CARD* card_sursa = find_card(&lc, card_number_source);
	CARD* card_dest = find_card(&lc, card_number_dest);
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
	while(p!=NULL)
    {
        if(strcmp(p->istoric, hist) == 0) {strcpy(p->istoric, "-1"); return;}
        p = p->nextHistory;
    }
}

void reverse_transaction(LC *lc, char card_number_source[30], char card_number_dest[30], unsigned int suma, FILE *fisier_out)
{
	CARD *card_source = find_card(&lc, card_number_source);
	CARD *card_dest = find_card(&lc, card_number_dest);
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

void unblock_card(LC **lc, char card_number[30])
{
	CARD *card = find_card(lc, card_number);
	strcpy(card->status, "ACTIVE");
	card->strike = 0;
}

void copiere_history(LC **lc, char card_number[30], history *hist)
{
	CARD *card = find_card(lc, card_number);
	card->History = hist;
}

void redimensionare_baza(LC **lc, int *poz_max, FILE* fisier_out)
{
	(*poz_max) = 0;
	(*lc)->nr_max_carduri = 2 * (*lc)->nr_max_carduri;
	LC *p = (*lc);

	while(p!=NULL)
	{
		CARD *card = p->infoLSC->nextCard;
		while(card!=NULL)
		{
			int poz = calculare_pozitie((*lc), card->card_number);
			CARD *card_nou = (CARD*)calloc(1, sizeof(CARD));
			CARD *card_aux = (CARD*)calloc(1, sizeof(CARD));

			card_aux->nextCard = card->nextCard;
			card_nou = add_info_card(card->card_number, card->pin, card->expiry_date, card->cvv, poz);
			card_nou->History = card->History;

			delete_card(lc, card->card_number);
			add_card(card_nou->card_number, card_nou->pin, card_nou->expiry_date, card_nou->cvv, lc , fisier_out, poz_max);

			//copiere_history(lc, card_nou->card_number, card_nou->History);
			card = card_aux -> nextCard;
		}
		p = p -> nextLC;
	}
}

int main()
{
	FILE* fisier_in = fopen("input.in", "r");
	FILE* fisier_out = fopen("output.out", "w");

	int nr_carduri_totale = 0, poz_max = 0;
	unsigned int nr_max_carduri;
	fscanf(fisier_in, "%d", &nr_max_carduri);

	LC *lc = (LC*)calloc(1, sizeof(LC));
	LSC *lsc = (LSC*)calloc(1, sizeof(LSC));
	lc->poz = 0;
	lc->nr_carduri_totale = 0;
	lc->nr_max_carduri = nr_max_carduri;
	lc->infoLSC = lsc;

	while(!feof(fisier_in))
	{
		char optiune[30], expiry_date[8];
		char card_number[30], pin[10], cvv[5];

		fscanf(fisier_in, "%s ", optiune);
		if(strcmp(optiune, "add_card") == 0)
		{
			fscanf(fisier_in, "%s %s %s %s", card_number, pin, expiry_date, cvv);
			int pozitie = calculare_pozitie(lc, card_number);
			if(verif_duplicate(lc, pozitie, card_number) != 1) nr_carduri_totale++;

			if(nr_carduri_totale > lc->nr_max_carduri) redimensionare_baza(&lc, &poz_max, fisier_out);
			add_card(card_number, pin, expiry_date, cvv, &lc, fisier_out, &poz_max);
		}
		else if(strcmp(optiune, "insert_card") == 0)
			{
				fscanf(fisier_in, "%s %s", card_number, pin);
				insert_card(&lc, card_number, pin, fisier_out);
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
				else show(lc, fisier_out, poz_max);
			}
			else if(strcmp(optiune, "delete_card") == 0)
			{
				nr_carduri_totale--;
				fscanf(fisier_in, "%s", card_number);
				delete_card(&lc, card_number);
			}
			else if(strcmp(optiune, "cancel") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				cancel(lc, card_number, fisier_out);
			}
			else if(strcmp(optiune, "unblock_card") == 0)
			{
				fscanf(fisier_in, "%s", card_number);
				unblock_card(&lc, card_number);
			}
			else if(strcmp(optiune, "reverse_transaction") == 0)
			{
				int suma;
				char card_number_source[30], card_number_dest[30];
				fscanf(fisier_in, "%s %s %d", card_number_source, card_number_dest, &suma);
				reverse_transaction(lc, card_number_source, card_number_dest, suma, fisier_out);
			}
			strcpy(optiune, "");
	}
	fclose(fisier_in);
	fclose(fisier_out);
	return 0;
}
