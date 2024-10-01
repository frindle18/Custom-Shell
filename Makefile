shell: main.c activities.c alias.c execute.c fg_bg.c hop.c iman.c input.c log.c neonate.c pipe.c proclore.c prompt.c redirection.c reveal.c seek.c signals.c system.c 
	gcc main.c activities.c alias.c execute.c fg_bg.c hop.c iman.c input.c log.c neonate.c pipe.c proclore.c prompt.c redirection.c reveal.c seek.c signals.c system.c -o shell
	
clean:
	rm -f shell
