#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <string>
#include "mpi.h"

using namespace std;

typedef struct Mesaj {
	int sursa;
	int destinatie;
	char mesaj[52];
} Mesaj;

/* 
 * citeste fisierul care contine topologia grafului si returneaza pentru fiecare rank 
 * un vector de vecini care e completat pana la numarul de noduri cu -1
 */
int* citireFisier(char* numeFisier, int rank, int numtasks) {
	ifstream in(numeFisier);
	string line;
	int nrLinie = 0;
	int* result = new int[numtasks];
	int index = 0;
	while (std::getline(in, line)) {
		if (nrLinie == rank) {
		 	int readRank;
		 	char* transformedLine = (char*)line.c_str();
		 	char* p;
			p = strtok (transformedLine," -\n");
			while (p != NULL) {
				int nodAdiacent = atoi (p);
				if (nodAdiacent != rank) {
					result[index] = nodAdiacent;
					index++;
				}
				p = strtok (NULL, " -\n");
			}
			for (int i = index; i < numtasks; i++) {
				result[i] = -1;
			}
		    in.close();
		    return result;
		}	
		else {
			nrLinie++;
		}
	}
}

/*
 * bfs de la sursa la destinatie - returneaza nextHop
 */
int bfs(int* muchii, int sursa, int destinatie, int numtasks) {
	bool* visited = new bool[numtasks];
	for (int i = 0; i < numtasks; i++) {
		visited[i] = false;
	}
	list<int> vecini;
	vecini.push_back(sursa);
	visited[sursa] = true;
	while(!vecini.empty()) {
		sursa = vecini.front();
		vecini.pop_front(); 
		for (int j = 0; j < numtasks * (numtasks - 1) && muchii[j] != -1; j = j + 2) {
			if (muchii[j + 1] == sursa && !visited[muchii[j]]) {
				if (muchii[j] == destinatie) {
					return sursa;
				}
				vecini.push_back(muchii[j]);
				visited[muchii[j]] = true;
			}
			if (muchii[j] == sursa && !visited[muchii[j+1]]) {
				if (muchii[j+1] == destinatie) {
					return sursa;
				}
				vecini.push_back(muchii[j+1]);
				visited[muchii[j+1]] = true;
			}
		}
	}
	return -1;
}

/*
 * completeaza tabela cu nextHop pentru fiecare rank si pentru fiecare destinatie
 */
void calculTabelaDeRutare(int* muchii, int rank, int numtasks, int* tabelaDeRutare) {
	for (int i = 0; i < numtasks; i++) {
		if (i == rank) {
			tabelaDeRutare[i] = -1;
		}
		else {
			tabelaDeRutare[i]  = bfs(muchii, i, rank, numtasks);
		}
	}
}

/*
 * pentru fiecare proces returnez vectorul de mesaje
 */
vector<Mesaj> citireFisierMesaje(char* numeFisier, int rank) {
	ifstream in(numeFisier);
	string line;
	vector<Mesaj> result;
	std::getline(in,line);
	int nrLinii = atoi(line.c_str());
	for (int j = 0; j < nrLinii; j++) {
	 	std::getline(in, line);
	 	char* transformedLine = (char*)line.c_str();
	 	int sizeOfLine = line.size();
	 	int i = 0;
	 	string sursa;
	 	while (i < sizeOfLine && transformedLine[i] != ' ') {
	 		sursa += transformedLine[i];
	 		i++;
	 	}
	 	while( transformedLine[i] == ' ') {
	 		i++;
	 	}
	 	if (atoi(sursa.c_str()) != rank) {
	 		continue;
	 	}
	 	Mesaj m;
	 	m.sursa = atoi(sursa.c_str());
	 	string destinatie;
	 	while (i < sizeOfLine && transformedLine[i] != ' ') {
	 		destinatie += transformedLine[i];
	 		i++;
	 	}
	 	while( transformedLine[i] == ' ') {
	 		i++;
	 	}
	 	if (destinatie.c_str()[0] == 'B') {
	 		m.destinatie = -1; 
		}
		else {
			m.destinatie = atoi(destinatie.c_str());
		}
	 	string mesaj;
	 	while (i < sizeOfLine) {
	 		mesaj += transformedLine[i];
	 		i++;
	 	}
		memset (m.mesaj,'\0',52);
		memcpy (m.mesaj, mesaj.c_str(), mesaj.size());
		if (m.destinatie != -1) {
			result.push_back(m);
		}
		else {
			vector<Mesaj>::iterator it;
  			it = result.begin();
			result.insert(it, m);
		}
	}	
	in.close();
	return result;
}

/*
 * transforma dintr-o structura Mesaj in char*
 */
char* StringfromMessage(Mesaj m) {

	char* result = new char[sizeof(Mesaj)];
	memcpy(result, &m, sizeof(Mesaj));
	return result;
}

/*
 * transforma char* intr-o structura Mesaj
 */
Mesaj StringtoMessage(char* message) {
	Mesaj m;
	memcpy(&m, message, sizeof(Mesaj));
	return m;
}

/*
 * trimit mesaj invalid la etapa2 spre destinatie
 */
void sendInvalidMessage(char* buffer, int destination, int rank) {
	Mesaj m;
	m.sursa = rank;
	m.destinatie = -2;
	memset (m.mesaj,'\0',52);
	buffer = StringfromMessage(m);
	MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, destination, 0, MPI_COMM_WORLD);
}

/*
 * trimit mesaj de deconectare la etapa2 spre destinatie
 */
void sendDisconnectMessage(char* buffer, int destination, int rank) {
	Mesaj m;
	m.sursa = rank;
	m.destinatie = -1;
	memset (m.mesaj,'\0',52);
	buffer = StringfromMessage(m);
	MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, destination, 0, MPI_COMM_WORLD);
}

/*
 * trimit mesaj de anuntare a inchiderii la etapa2 spre destinatie
 */
void sendClosedMessage(char* buffer, int destination, int rank) {
	Mesaj m;
	m.sursa = rank;
	m.destinatie = -3;
	memset (m.mesaj,'\0',52);
	buffer = StringfromMessage(m);
	MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, destination, 0, MPI_COMM_WORLD);
}

/*
 * trimit mesaj wakeup la etapa3 spre destinatie
 */
void sendWakeupMessage(char* buffer, int rank, int destination) {
	MPI_Request req;
	Mesaj wakeup;
	wakeup.sursa = rank;
	wakeup.destinatie = -5;
	buffer = StringfromMessage(wakeup);
	MPI_Isend(buffer, sizeof(Mesaj), MPI_CHAR, destination, 2, MPI_COMM_WORLD, &req);
}

/*
 * trimit mesaj din tree algoritm la etapa3 spre destinatie
 */
void sendTreeMessage(char* buffer, int rank, int destination, int leader, int semiLeader) {
	MPI_Request req;
	Mesaj deTrimis;
	deTrimis.sursa = leader;
	deTrimis.destinatie = semiLeader;
	buffer = StringfromMessage(deTrimis);
	MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, destination, 3, MPI_COMM_WORLD);
}

/*
 * calculeaza primii 2 cei mai mici indici din cei 4 si ii salvez in leader si semiLeader
 */
void calculLideri(int& leader, int& semiLeader, int sursa, int destinatie, int rank) {
	vector<int> numbers;
	numbers.push_back(leader);
	numbers.push_back(semiLeader);
	numbers.push_back(sursa);
	numbers.push_back(destinatie);
	vector<int> unique;
	for (int i = 0; i < numbers.size() - 1; i++) {
		bool unic = true;
		for (int j = i + 1; j < numbers.size(); j++) {
			if (numbers[i] == numbers[j]) {
				unic = false;
			}
		}
		if (unic) {
			unique.push_back(numbers[i]);
		}
	}
	unique.push_back(numbers[numbers.size() - 1]);

	bool sw = true;
	do {
		sw = true;
		for (int i = 0; i < unique.size() - 1; i++) {
			if (unique[i] > unique[i+1]) {
				int aux = unique[i];
				unique[i] = unique[i+1];
				unique[i+1] = aux;
				sw = false;
			}
		}
	} while(!sw);

	leader = unique[0];
	semiLeader = unique[1];
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cout << "Numar gresit de parametri\n";
	}

	int  numtasks, rank; 
	MPI_Status status;
	MPI_Request req;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	int* listaDeAdiacenta;
	listaDeAdiacenta = citireFisier(argv[1], rank, numtasks);
	vector<Mesaj> toSend;
	char* buffer = new char[sizeof(Mesaj)];
	bool* open = new bool[numtasks];
	bool* foreverClosed = new bool[numtasks];

	for (int i = 0; i < numtasks; i++) {
		open[i] = true;
		foreverClosed[i] = false;
	}

	int pozitie = 0;
	int parinte = -1;

	int* bufferTrimis = new int[numtasks * (numtasks - 1)];
	int* bufferPrimit = new int[numtasks * (numtasks - 1)];
	int* tabelaDeRutare = new int[numtasks];

	for (int i = 0;  i < numtasks * (numtasks - 1); i++) {
		bufferTrimis[i] = -1;
		bufferPrimit[i] = -1;
	}

	if (rank == 0) {
		parinte = -1;
		for (int i = 0; i < numtasks; i++) {
		  if (listaDeAdiacenta[i] != -1) {
		  	// 0 este initiator - trimite catre toti vecinii
		    MPI_Send(bufferTrimis, numtasks * (numtasks - 1), MPI_INT, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
		  }
		}
		// primeste de la toata lumea rezultatele
		for (int i = 0; i < numtasks; i++) {
		  if (listaDeAdiacenta[i] != -1) {
		    MPI_Recv(bufferPrimit, numtasks * (numtasks - 1), MPI_INT, listaDeAdiacenta[i], 0, MPI_COMM_WORLD, &status);
		    for (int j = 0; j < numtasks * (numtasks - 1) && bufferPrimit[j] != -1; j = j + 2) {
		      bufferTrimis[pozitie] = bufferPrimit[j];
		      pozitie = pozitie + 1;
		      bufferTrimis[pozitie] = bufferPrimit[j + 1];
		      pozitie = pozitie + 1;
		    }
		  }
		}
		for (int i = 0; i < numtasks; i++) {
		  if (listaDeAdiacenta[i] != -1) {
		    MPI_Send(bufferTrimis, numtasks * (numtasks - 1), MPI_INT, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
		  }
		}

		calculTabelaDeRutare(bufferTrimis, rank, numtasks, tabelaDeRutare);

		// calculez matrice de adiacenta pe baza topologiei si o afisez in fisier
		bool** matriceDeAdiacenta = new bool*[numtasks];
		for (int i = 0; i < numtasks; i++) {
			matriceDeAdiacenta[i] = new bool[numtasks];
			for (int j = 0; j < numtasks; j++) {
				matriceDeAdiacenta[i][j] = false;
			}
		}

		for (int k = 0; k < numtasks * (numtasks - 1) && bufferTrimis[k] != -1; k = k + 2) {
			matriceDeAdiacenta[bufferTrimis[k]][bufferTrimis[k+1]] = true;	
			matriceDeAdiacenta[bufferTrimis[k+1]][bufferTrimis[k]] = true;			
		}

		ofstream etapa1;
		etapa1.open ("Etapa1.out", std::ofstream::out | std::ofstream::app);
		for (int i = 0; i < numtasks; i++) {
			etapa1 << rank << ": " << tabelaDeRutare[i] << " " << i << "\n";
		}
		for (int i = 0; i < numtasks; i++) {
			etapa1 << rank << ":";
			for (int j = 0; j < numtasks; j++) {
				etapa1 <<  " " << matriceDeAdiacenta[i][j];
			}
			etapa1 << "\n";
		}
		etapa1.close();

		// aici incepe etapa 2
		toSend = citireFisierMesaje(argv[2], rank);

		if (toSend.size() != 0 && toSend[0].destinatie == -1) {
			// trimite broadcast
			buffer = StringfromMessage(toSend[0]);
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
			}
		}
		else {
			// primeste broadcast
			MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			int nodSursa = status.MPI_SOURCE;
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				if (listaDeAdiacenta[i] == nodSursa) {
					continue;
				}
				MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
			}
		}

		// trimit in jos pe arbore un mesaj valid (daca am mesaj de trimis) sau invalid
		// maxim 1 mesaj odata intre nodul curent si alt nod

		while(true) {
			bool existaProcesDeschis = false;
			for (int i = 0; i < numtasks; i++) {
				if (open[i]) {
					existaProcesDeschis = true;
				}
			} 

			// daca am primit mesaje de disconnect de la toata lumea anunt inchiderea procesului curent
			int allClosed = true;
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				if (!foreverClosed[listaDeAdiacenta[i]]) {
					allClosed = false;
				}
			}

			if ((!existaProcesDeschis && toSend.size() == 0) || allClosed) {
				for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
					if (!foreverClosed[listaDeAdiacenta[i]]) {
						sendClosedMessage(buffer, listaDeAdiacenta[i], rank);
					}
				}
				break;
			}

			// daca nu mai am mesaje pe procesul curent trimit mesaj broadcast de deconectare
			if (toSend.size() == 0 && open[rank]) {
				open[rank] = false;
				for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
					if (!foreverClosed[listaDeAdiacenta[i]]) {
						sendDisconnectMessage(buffer, listaDeAdiacenta[i], rank);
					}
				}
			}
			else {
				// daca primul e broadCast
				if (toSend.size() != 0 && toSend[0].destinatie == -1) {
					for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
						// daca nodul din lista de adiacenta e calea pe care a venit bcastul trimit invalid
						if (listaDeAdiacenta[i] == toSend[0].sursa) {
							if (!foreverClosed[listaDeAdiacenta[i]]) {
								sendInvalidMessage(buffer, listaDeAdiacenta[i], rank);
							}
						}
						else if (tabelaDeRutare[toSend[0].sursa] == listaDeAdiacenta[i]) {
							if (!foreverClosed[listaDeAdiacenta[i]]) {
								sendInvalidMessage(buffer, listaDeAdiacenta[i], rank);
							}
						}
						// trimit broadcastul mai departe
						else {
							if (!foreverClosed[listaDeAdiacenta[i]]) {
								buffer = StringfromMessage(toSend[0]);
								MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
							}
						}
					}
					toSend.erase(toSend.begin());
				}
				else {
					// trimit tuturor copiilor mesaje
					for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
						int j = 0;
						for (j = 0; j < toSend.size(); j++) {
							// daca nexthop e vecin cu mine ii trimit mesaj valid
							if (toSend[j].destinatie != -1 && 
								tabelaDeRutare[toSend[j].destinatie] == listaDeAdiacenta[i]) {
								if (!foreverClosed[listaDeAdiacenta[i]]) {
									buffer = StringfromMessage(toSend[j]);
									MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
								}
								break;
							}
						}
						
						if (j == toSend.size()) {
							if (!foreverClosed[listaDeAdiacenta[i]]) {
								sendInvalidMessage(buffer, listaDeAdiacenta[i], rank);
							}	
						}
						// sterg mesajul de mai sus - cel cu nexthop
						else {
							toSend.erase(toSend.begin() + j);
						}
					}
				}
			}
			// primesc mesaje de la vecini
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				if (foreverClosed[listaDeAdiacenta[i]]) {
					continue;
				}	

				MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

				int nodSursa = status.MPI_SOURCE;
				Mesaj m = StringtoMessage(buffer);

				ofstream etapa2;
				etapa2.open ("Etapa2.out", std::ofstream::out | std::ofstream::app);

				// mesaj de anuntare ca nodul se inchide - broadcastul de terminare
				if (m.destinatie == -3) {
					foreverClosed[nodSursa] = true;
				}
				// mesaj invalid
				else if (m.destinatie == -2) {
				}
				// mesaj de broadcast 
				else if (m.destinatie == -1) {
					// de deconectare
					if (m.mesaj[0] == '\0') {
						// daca acum se inchide trimit mai departe
						if (open[m.sursa]) {
							toSend.push_back(m);
							open[m.sursa] = false;
						}
					}
					// mesaj de transmis mai departe
					else {
						toSend.push_back(m);
						etapa2 << rank << ": " << m.sursa << " " << nodSursa << " -1 -1 " << m.mesaj << "\n";				
					}
				}
				// rank e doar next hop
				else if (m.destinatie != rank) {
					toSend.push_back(m);
					etapa2 << rank << ": " << m.sursa << " " << nodSursa << " " << tabelaDeRutare[m.destinatie] << 
						" " << m.destinatie << " " << m.mesaj <<  "\n";
				}
				// rand e destinatia
				else {
					etapa2 << rank << ": " << m.sursa << " " << nodSursa << " " << tabelaDeRutare[m.destinatie] << 
						" " << m.destinatie << " " << m.mesaj << "\n";
				}
				etapa2.close();
			
			}
		}

		// aici incepe etapa 3

		bool ws = false;
		int wr = 0;
		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			wr++;
		}
		int count = 0;

		// pentru fiecare frunza trimit send wakeup
		if (listaDeAdiacenta[0] == parinte && listaDeAdiacenta[1] == -1) {
			sendWakeupMessage(buffer, rank, parinte);
			MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, parinte, 2, MPI_COMM_WORLD, &status);
		} 
		else {
			// daca nu e frunza extind mesajul de wakeup prin broadcast
			while (count < wr) {
				MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 2,  MPI_COMM_WORLD, &status);
				count++;
				if (ws == false) {
					ws = true;
					for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
						sendWakeupMessage(buffer, rank, listaDeAdiacenta[i]);
					}
				}  
			}
		}

		// alegere a liderului
		int leader = rank;
		int semiLeader = 1000;

		count = 0;
		bool* received = new bool[numtasks];
		for (int i = 0; i < numtasks; i++) {
			received[i] = false;
		}

		while (count != wr - 1) {
			count++;
			MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 3,  MPI_COMM_WORLD, &status);
			int nodPrimit = status.MPI_SOURCE;
			received[nodPrimit] = true;
			Mesaj mesajPrimit = StringtoMessage(buffer);

			calculLideri(leader, semiLeader, mesajPrimit.sursa, mesajPrimit.destinatie, rank);
		} 

		int deTrimis = -1;
		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			if (received[listaDeAdiacenta[i]]) {
				continue;
			}	
			deTrimis = listaDeAdiacenta[i];
		}
		sendTreeMessage(buffer, rank, deTrimis, leader, semiLeader);
		MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, deTrimis, 3,  MPI_COMM_WORLD, &status);
		Mesaj mesajPrimit = StringtoMessage(buffer);

		calculLideri(leader, semiLeader, mesajPrimit.sursa, mesajPrimit.destinatie,rank);

		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			if (listaDeAdiacenta[i] == deTrimis) {
				continue;
			}
			sendTreeMessage(buffer, rank, listaDeAdiacenta[i], leader, semiLeader);
		}

		ofstream etapa3;
		etapa3.open ("Etapa3.out", std::ofstream::out | std::ofstream::app);
		etapa3 << rank << ": " << leader << " " << semiLeader << "\n";  
		etapa3.close();

	}
	else {
		
		// primeste de la parinte si seteaza parintele
		MPI_Recv(bufferPrimit, numtasks * (numtasks - 1), MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		parinte = status.MPI_SOURCE;
		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			if (listaDeAdiacenta[i] == parinte) {
				continue;
			}
			// trimite catre ceilalti vecini
			MPI_Isend(bufferPrimit, numtasks * (numtasks - 1), MPI_INT, listaDeAdiacenta[i], 0, MPI_COMM_WORLD, &req);
		}
		// fiecare nod primeste de la vecinii sai cu exceptia parintelui si retine muchiile
		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			if (listaDeAdiacenta[i] == parinte) {
				continue;
			}
			MPI_Recv(bufferPrimit, numtasks * (numtasks - 1), MPI_INT, listaDeAdiacenta[i], 0, MPI_COMM_WORLD, &status);
			for (int j = 0; j < numtasks * (numtasks - 1) && bufferPrimit[j] != -1; j = j + 2) {
				bufferTrimis[pozitie] = bufferPrimit[j];
				pozitie = pozitie + 1;
				bufferTrimis[pozitie] = bufferPrimit[j + 1];
				pozitie = pozitie + 1;
			}
		}

		// adaug legatura intre parinte si rankul propriu si trimit la parinte
		bufferTrimis[pozitie] = parinte;
		pozitie = pozitie + 1;
		bufferTrimis[pozitie] = rank;
		pozitie = pozitie + 1;

		MPI_Send(bufferTrimis, numtasks * (numtasks - 1), MPI_INT, parinte, 0, MPI_COMM_WORLD);

		// primeste topologia
		MPI_Recv(bufferPrimit, numtasks * (numtasks - 1), MPI_INT, parinte, 0, MPI_COMM_WORLD, &status);
		
		// resetez lista de adiacenta
		for (int i = 0; i < numtasks; i++) {
			listaDeAdiacenta[i] = -1;
		}

		// actualizez bufferTrimis care retine de fapt topologia in fiecare nod partiala sau acum, totala
		for(int i = 0; i < numtasks * (numtasks - 1); i++) {
			bufferTrimis[i] = bufferPrimit[i];
		}

		// introduc in lista de adiacenta doar copiii + parintele eliminand redundantele
		int index = 0;
		listaDeAdiacenta[index] = parinte;
		index++;
		for (int i = 0; i < numtasks * (numtasks - 1) && bufferTrimis[i] != -1; i = i + 2) {
			if (bufferTrimis[i] == rank && bufferTrimis[i+1] != parinte) {
				listaDeAdiacenta[index] = bufferTrimis[i+1];
				index++;
				// trimit copiilor topologia mai departe in broadcast
				MPI_Send(bufferTrimis, numtasks * (numtasks - 1), MPI_INT, bufferTrimis[i+1], 0, MPI_COMM_WORLD);
			}
		}

		calculTabelaDeRutare(bufferTrimis, rank, numtasks, tabelaDeRutare);
		
		ofstream etapa1;
		etapa1.open ("Etapa1.out", std::ofstream::out | std::ofstream::app);
		for (int i = 0; i < numtasks; i++) {
			etapa1 << rank << ": " << tabelaDeRutare[i] << " " << i << "\n";
		}
		etapa1.close();

		// aici incepe etapa 2 
		toSend = citireFisierMesaje(argv[2], rank);

		if (toSend.size() != 0 && toSend[0].destinatie == -1) {
			// trimite broadcast initial daca broadcastul se afla printre mesaje
			buffer = StringfromMessage(toSend[0]);
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
			}
			toSend.erase(toSend.begin());
		}
		else {
			// primeste broadcast initial
			MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			int nodSursa = status.MPI_SOURCE;
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				if (listaDeAdiacenta[i] == nodSursa) {
					continue;
				}
				MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
			}
		}

		/* pentru un nod intermediar introduc mesaje invalide pentru a nu avea blocaje precum 
		 * 2 procese dau send simultan unu catre altul sau 2 procese dau recv simultan unu catre altul
		 * primesc de la parinte, trimit tuturor copiilor, primesc de la copii, trimit parintelui
		 */
		while(true) {
			
			bool existaProcesDeschis = false;
			for (int i = 0; i < numtasks; i++) {
				if (open[i]) {
					existaProcesDeschis = true;
				}
			}

			// receive de la parinte
			if (!foreverClosed[parinte]) {
				MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, parinte, 0, MPI_COMM_WORLD, &status);

				Mesaj m = StringtoMessage(buffer);

				ofstream etapa2;
				etapa2.open ("Etapa2.out", std::ofstream::out | std::ofstream::app);

				// mesaj ca parinte s-a inchis - broadcast de terminare
				if (m.destinatie == -3) {
					foreverClosed[parinte] = true;
				}
				// mesaj invalid - il ignor
				else if (m.destinatie == -2) {
				}
				// mesaj de broadcast 
				else if (m.destinatie == -1) {
					// de deconectare
					if (m.mesaj[0] == '\0') {
						// daca acum se inchide adaug mesaj de trimis
						if (open[m.sursa]) {
							toSend.push_back(m);
							open[m.sursa] = false;
						}
					}
					// mesaj de transmis mai departe
					else {
						toSend.push_back(m);
						etapa2 << rank << ": " << m.sursa << " " << parinte << " -1 -1 " << m.mesaj << "\n";				
					}
				}
				// daca rank e nod intermediar adaug in vectorul de mesaje
				else if (m.destinatie != rank) {
					toSend.push_back(m);
					etapa2 << rank << ": " << m.sursa << " " << parinte << " " << tabelaDeRutare[m.destinatie] << 
					" " << m.destinatie << " " << m.mesaj << "\n";
				}
				// daca rank e destinatia doar afisez
				else {
					etapa2 << rank << ": " << m.sursa << " " << parinte << " " << tabelaDeRutare[m.destinatie] << 
					" " << m.destinatie << " " << m.mesaj <<  "\n";
				}

				etapa2.close();
			}

			// trimite mesaje la toti copiii - cauta mesaje valide care sa fie nevoite sa mearga pe acea cale
			// daca nu gaseste trimit mesaj invalid, dar trebuie sa trimit ceva pentru ca la celalalt capat e un recv
			bool bcast = false;
			bool disconnect = false;

			int allClosed = true;
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				if (!foreverClosed[listaDeAdiacenta[i]]) {
					allClosed = false;
				}
			}

			// inchide nodul curent si trimit broadcast de terminare
			if ((!existaProcesDeschis && toSend.size() == 0) || allClosed) {
				for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
					if (!foreverClosed[listaDeAdiacenta[i]]) {
						sendClosedMessage(buffer, listaDeAdiacenta[i], rank);
					}
				}
				break;
			}
			// daca primul e broadCast
			if (toSend.size() != 0 && toSend[0].destinatie == -1) {
				bcast = true;
				for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
					// in aceasta etapa nu ii trimit nimic parintelui
					if (listaDeAdiacenta[i] == parinte) {
						continue;
					}
					// daca broadcastul a venit de la listaDeAdiacenta[i] - trimit invalid
					if (listaDeAdiacenta[i] == toSend[0].sursa || listaDeAdiacenta[i] == tabelaDeRutare[toSend[0].sursa]) {
						if (!foreverClosed[listaDeAdiacenta[i]]) {
							sendInvalidMessage(buffer, listaDeAdiacenta[i], rank);
						}				
					}
					else {
						// trimit broadcastul
						if (!foreverClosed[listaDeAdiacenta[i]]) {
							buffer = StringfromMessage(toSend[0]);
							MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
						}
					}
				}
			}
			else {
				// trimit tuturor copiilor mesaje
				if (toSend.size() == 0 && open[rank]) {
					disconnect = true;
					open[rank] = false;
					for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
						// in aceasta etapa nu ii trimit nimic parintelui
						if (listaDeAdiacenta[i] == parinte) {
							continue;
						}
						if (!foreverClosed[listaDeAdiacenta[i]]) {
							sendDisconnectMessage(buffer, listaDeAdiacenta[i], rank);
						}
					}
				}
				else {
					for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
						// in aceasta etapa nu ii trimit nimic parintelui
						if (listaDeAdiacenta[i] == parinte) {
							continue;
						}
						int j = 0;
						for (j = 0; j < toSend.size(); j++) {
							// daca nexthop e vecin cu mine ii trimit mesaj valid
							if (toSend[j].destinatie != -1 && 
								tabelaDeRutare[toSend[j].destinatie] == listaDeAdiacenta[i]) {
								if (!foreverClosed[listaDeAdiacenta[i]]) {
									buffer = StringfromMessage(toSend[j]);
									MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, listaDeAdiacenta[i], 0, MPI_COMM_WORLD);
								}
								break;
							}
						}
						
						if (j == toSend.size()) {	
							if (!foreverClosed[listaDeAdiacenta[i]]) {
								sendInvalidMessage(buffer, listaDeAdiacenta[i], rank);
							}
						}
						else {
							toSend.erase(toSend.begin() + j);
						}
					}
				}
			}

			// primesc mesaje de la copii
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				if (listaDeAdiacenta[i] == parinte) {
					continue;
				}
				if (foreverClosed[listaDeAdiacenta[i]]) {
					continue;
				}
				
				MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
				int nodSursa = status.MPI_SOURCE;
				Mesaj m = StringtoMessage(buffer);

				ofstream etapa2;
				etapa2.open ("Etapa2.out", std::ofstream::out | std::ofstream::app);

				// am primit mesaj de inchidere a nodSursa
				if (m.destinatie == -3) {
					foreverClosed[nodSursa] = true;
				}
				// mesaj invalid
				else if (m.destinatie == -2) {
				}
				// mesaj de broadcast 
				else if (m.destinatie == -1) {
					// de deconectare
					if (m.mesaj[0] == '\0') {
						// daca se termina acum adaug in vector
						if (open[m.sursa]) {
							toSend.push_back(m);
							open[m.sursa] = false;
						}
					}
					// mesaj de transmis mai departe
					else {
						toSend.push_back(m);
						etapa2 << rank << ": " << m.sursa << " " << nodSursa << " -1 -1 " << m.mesaj <<  "\n";		
					}
				}
				// daca rank nu e destinatie, ci nexthop pentru mesaj
				else if (m.destinatie != rank) {
					toSend.push_back(m);
					etapa2 << rank << ": " << m.sursa << " " << nodSursa << " " << tabelaDeRutare[m.destinatie] << 
					" " << m.destinatie << " " << m.mesaj << "\n";
				}
				// daca rank e destinatia
				else {
					etapa2 << rank << ": " << m.sursa << " " << nodSursa << " " << tabelaDeRutare[m.destinatie] << 
					" " << m.destinatie << " " << m.mesaj << "\n";
				}
				etapa2.close();
			}

			// testez din nou daca sunt deconectate toate procesele pentru a lansa broadcastul de terminare
			allClosed = true;
			for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
				if (!foreverClosed[listaDeAdiacenta[i]]) {
					allClosed = false;
				}
			}

			if ((!existaProcesDeschis && toSend.size() == 0) || allClosed) {
				for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++){
					if (!foreverClosed[listaDeAdiacenta[i]]) {
						sendClosedMessage(buffer, listaDeAdiacenta[i], rank);
					}
				}
				break;
			}

			// daca s-a deconectat mai sus trimit si parintelui in etapa destinata acestuia
			if (disconnect) {
				if (!foreverClosed[parinte]) {
					sendDisconnectMessage(buffer, parinte, rank);
				}
			}
			// trimit parintelui mesaj 
			else if (bcast) {
				// ii trimit parintelui primul mesaj din toSend
				if (parinte == toSend[0].sursa || tabelaDeRutare[toSend[0].sursa] == parinte) {
					if (!foreverClosed[parinte]) {
						sendInvalidMessage(buffer, parinte, rank);
					}
				}
				else {
					if (!foreverClosed[parinte]) {
						buffer = StringfromMessage(toSend[0]);
						MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, parinte, 0, MPI_COMM_WORLD);
					}
				}
				toSend.erase(toSend.begin() + 0);
			}
			else {
				int k = 0;
				for (k = 0; k < toSend.size(); k++) {
					// daca parintele e nexthop pentru vreun mesaj ii trimit mesaj valid
					if (toSend[k].destinatie != -1 && 
						tabelaDeRutare[toSend[k].destinatie] == parinte) {
							if (!foreverClosed[parinte]) {
								buffer = StringfromMessage(toSend[k]);
								MPI_Send(buffer, sizeof(Mesaj), MPI_CHAR, parinte, 0, MPI_COMM_WORLD);
							}
							break;
					}
				}
				
				if (k == toSend.size()) {
					if (!foreverClosed[parinte]) {
						sendInvalidMessage(buffer, parinte, rank);
					}
				}
				else {
					toSend.erase(toSend.begin() + k);
				}
			}
		}

		// aici incepe etapa 3
		bool ws = false;
		int wr = 0;
		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			wr++;
		}
		int count = 0;

		// pentru fiecare frunza trimit send wakeup
		if (listaDeAdiacenta[0] == parinte && listaDeAdiacenta[1] == -1) {
			sendWakeupMessage(buffer, rank, parinte);
			MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, parinte, 2, MPI_COMM_WORLD, &status);
		} 
		else {
			while (count < wr) {
				MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
				count++;
				if (ws == false) {
					ws = true;
					for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
						sendWakeupMessage(buffer, rank, listaDeAdiacenta[i]);
					}
				}  
			}
		}

		int leader = rank;
		int semiLeader = 1000;

		count = 0;
		bool* received = new bool[numtasks];
		for (int i = 0; i < numtasks; i++) {
			received[i] = false;
		}

		while (count != wr - 1) {
			count++;
			MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, MPI_ANY_SOURCE, 3,  MPI_COMM_WORLD, &status);
			int nodPrimit = status.MPI_SOURCE;
			received[nodPrimit] = true;
			Mesaj mesajPrimit = StringtoMessage(buffer);

			calculLideri(leader, semiLeader, mesajPrimit.sursa, mesajPrimit.destinatie, rank);
		} 

		int deTrimis = -1;
		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			if (received[listaDeAdiacenta[i]]) {
				continue;
			}	
			deTrimis = listaDeAdiacenta[i];
		}
		sendTreeMessage(buffer, rank, deTrimis, leader, semiLeader);
		MPI_Recv(buffer, sizeof(Mesaj), MPI_CHAR, deTrimis, 3,  MPI_COMM_WORLD, &status);
		Mesaj mesajPrimit = StringtoMessage(buffer);

		calculLideri(leader, semiLeader, mesajPrimit.sursa, mesajPrimit.destinatie, rank);

		for (int i = 0; i < numtasks && listaDeAdiacenta[i] != -1; i++) {
			if (listaDeAdiacenta[i] == deTrimis) {
				continue;
			}
			sendTreeMessage(buffer, rank, listaDeAdiacenta[i], leader, semiLeader);
		}

		ofstream etapa3;
		etapa3.open ("Etapa3.out", std::ofstream::out | std::ofstream::app);
		etapa3 << rank << ": " << leader << " " << semiLeader << "\n";  
		etapa3.close();
	}

	MPI_Finalize();
}