# Bank-Management-Application

This is one of my University projects. 

It simulates an ATM and its database. The database is done using linked lists. Each card in the database has a history of operations, pin, balance, cvv and a lot of other informations.

There are a lot of administrator or user functions like:

> add-card: Adds a card in the database
> insert-card: Simulates a human inserting the card in ATM
> recharge: Adds money to the last inserted card
> cash_withdrawal: Withdraws money from the last inserted card
> balance_inquiry: Queries the balance of the last inserted card
> pin_change: Changes the PIN
> transfer_funds: Transfers money from the last inserted card to another given card number
> show_card: Shows all the info of the current inserted card, including the history of operations
> delete_card: Delete a card from the database
> cancel: Removes the card from the ATM
> unblock_card: Unblocks the card in case the PIN was wrong 3 times in a row
> reverse_transaction: Reverses a transaction and updates the history
