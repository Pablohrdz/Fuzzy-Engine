#include <iostream>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

enum MFType { TRIANG, TRAP, SAT, GAUSS };

/*** Norms ***/
//Vector version
double fAnd(vector<double> args);
double fOr(vector<double> args);

//Two arguments version
double fAnd(double a, double b);
double fOr(double a, double b);


//Membership Functions
double triangmf(double left, double center, double right, double x); //Triangular
double trapmf(double lowLeft, double upLeft, double upRight, double lowRight, double x); //π-Function (trapecio)
double satmf(double up, double down, double x); //Saturation
double gaussianmf(double center, double width, double x);

//Defuzzification Methods
//Se usará el máximo de las reglas disparadas para cambiar a la máquina de estados difusa (ese valor sale directamente del
//output de rules.inferMamdani(x) )

struct FuzzySet
{
	string name;
	MFType type;
	vector<double> params;

	FuzzySet(string n): name(n) {}

	double eval(double x)
	{
		double res = 0;

		switch(type)
		{
			case TRIANG:
				if(params.size() == 3)				res = triangmf(params[0], params[1], params[2], x);
			break;

			case TRAP:
				if(params.size() == 4)				res = trapmf(params[0], params[1], params[2], params[3], x);
			break;

			case SAT:
				if(params.size() == 2)				res = satmf(params[0], params[1], x);
			break;

			case GAUSS:
				if(params.size() == 2)				res = gaussianmf(params[0], params[1], x);
			break;

			default:
				cout << "No adequate MF" << endl;
			break;
		}

		return res;
	}

	void setMF(MFType t, double *args, int size)
	{
		type = t;

		for(int i = 0; i < size; i++)
		{
			params.push_back(args[i]);
		}
	}
};

/* Rules are added according to the next (not validated) format:

	IF InputFuzzySetName Norm THEN OutputFuzzySetName

	where IF and THEN are those exact strings, 
	InputFuzzySetName is the name of an input FuzzySet struct,
	Norm is either a "OR", or an "AND" string,
	OutputFuzzySetName is the name of an output FuzzySet struct.

	Norms are evaluated from left to right, so ORs should come first, 
	and ANDs should come ONLY after all the ORs.
*/

struct Rules
{
	vector<string> rules;

	void addRule(string r)
	{
		//Existence of the rule is not validated
		rules.push_back(r);
	}

	/* Inferencia Mamdani. Como en Matlab, una regla sólo puede contener ANDs, o sólo puede contener ORs (por el momento) */
	map<string, double> inferMamdani(map<string, double> inputMembershipValues)
	{
		multimap<string, double> outputMembershipValues;


		//j = 3 para ignorar el "IF "
		for(int i = 0, j = 3; i < rules.size(); i++)
		{
			j = 3;
			string rule = rules[i];
			string fuzzySetName = "";

			cout << "IF" << endl;

			bool andFlag = false, orFlag = false;
			double accum = 0;

			while(fuzzySetName != "THEN" && fuzzySetName != "then")
			{
				fuzzySetName = "";
				//cout << "accum - " << accum << endl;

				while(rule[j] != ' ')
				{
					fuzzySetName += rule[j++];
				}

				cout << "fuzzySetName - " << fuzzySetName << endl;

				if(fuzzySetName == "AND" || fuzzySetName == "and")			andFlag = true;
				else if(fuzzySetName == "OR" || fuzzySetName == "or")		orFlag = true;
				else 
				{
					//Si se encuentra el fuzzy set en el mapa
					if(inputMembershipValues.find(fuzzySetName) != inputMembershipValues.end())
					{
						double currentValue = inputMembershipValues[fuzzySetName];

						if(andFlag)	
						{
							andFlag = false;
							accum = fAnd(currentValue, accum);
						}

						else if(orFlag)
						{
							orFlag = false;
							accum = fOr(currentValue, accum);
						}

						//Sólo se va a entrar una vez a este else (para el primer valor de la regla, antes de cualquier AND u OR)
						else
						{
							accum = currentValue;
						}
					}
				}

				j++;
			}

			//Salió del THEN
			fuzzySetName = "";

			while(j < rule.length())
			{
				fuzzySetName += rule[j];
				j++;
			}

			cout << "fuzzySetName - " << fuzzySetName << endl;
			//cout << "value - " << accum << endl;

			//Insertar el resultado en el multimapa, asociando el output de la regla con el output fuzzy set.
			outputMembershipValues.insert(pair<string, double>(fuzzySetName, accum));

		}

		//Sacar el máximo de los valores de salida para cada output fuzzy set y meterlos en el mapa que regresa la función
		map<string, double> output;
 
		//Multimap ordena todo por orden alfabético
		string currentSet = "";
		double maximum = 0;

		
		for(multimap<string, double>::iterator itr = outputMembershipValues.begin(); itr != outputMembershipValues.end(); ++itr)
		{
			//Si cambia de set
			if((*itr).first != currentSet)			
			{
				maximum = 0;
				currentSet = (*itr).first;
			}

			//Calcular el máximo para cada output fuzzy set
			maximum = max(maximum, (*itr).second);
			//Guardar el valor máximo para este fuzzy set
			output[currentSet] = maximum;
		}

		//Añadir el máximo del último fuzzy set (el for se sale antes de guardarlo)
		output[currentSet] = maximum;

		return output;
	}

};


int main()
{		
	double TIEMPO_TOTAL = 300;

	/*************************** Controlador para el skill del jugador ***************************/
	
	// Crisp Inputs 
	double crispInputScore = 75;
	double crispInputTiempoVida = 7.32;
	double crispInputVidasRestantes = 57;

	// Input Fuzzy Sets 
	FuzzySet vidas_restantes_pocas("Vidas_Restantes_Pocas");
	FuzzySet vidas_restantes_promedio("Vidas_Restantes_Promedio");
	FuzzySet vidas_restantes_muchas("Vidas_Restantes_Muchas");

	FuzzySet score_bajo("Score_Bajo");
	FuzzySet score_promedio("Score_Promedio");
	FuzzySet score_alto("Score_Alto");

	FuzzySet tiempo_de_vida_bajo("Tiempo_De_Vida_Bajo");
	FuzzySet tiempo_de_vida_promedio("Tiempo_De_Vida_Promedio");
	FuzzySet tiempo_de_vida_alto("Tiempo_De_Vida_Alto");


	// Output Fuzzy Sets 
	FuzzySet skill_jugador_bajo("Skill_Jugador_Bajo");
	FuzzySet skill_jugador_promedio("Skill_Jugador_Promedio");
	FuzzySet skill_jugador_alto("Skill_Jugador_Alto");


	// Membership Functions 

	//Vidas Restantes
	double vidas_restantes_pocas_args[] = {1, 3};
	vidas_restantes_pocas.setMF(SAT, vidas_restantes_pocas_args, 2);
	
	double vidas_restantes_promedio_args[] = {1, 5, 10};
	vidas_restantes_promedio.setMF(TRIANG, vidas_restantes_promedio_args, 3);

	double vidas_restantes_muchas_args[] = {10, 3};
	vidas_restantes_muchas.setMF(SAT, vidas_restantes_muchas_args, 2);

	//Score (normalizado a 100)
	double score_bajo_args[] = {0, 50};
	score_bajo.setMF(SAT, score_bajo_args, 2);

	double score_promedio_args[] = {0, 50, 100};
	score_promedio.setMF(TRIANG, score_promedio_args, 3);

	double score_alto_args[] = {100, 50};
	score_alto.setMF(SAT, score_alto_args, 2);


	//Tiempo de Vida
	double tiempo_de_vida_bajo_args[] = {0, 50}; //(Tiempo total por nivel / 6) -> 300/6
	tiempo_de_vida_bajo.setMF(SAT, tiempo_de_vida_bajo_args, 2);

	double tiempo_de_vida_promedio_args[] = {0, 150, TIEMPO_TOTAL};
	tiempo_de_vida_promedio.setMF(TRIANG, tiempo_de_vida_promedio_args, 3);

	double tiempo_de_vida_alto_args[] = {TIEMPO_TOTAL, 0};
	tiempo_de_vida_alto.setMF(SAT, tiempo_de_vida_alto_args, 2);


	cout << "Fuzzification" << endl;

	// Fuzzification 
	map<string, double> inputMembershipValuesSkill;

	//Vidas restantes
	inputMembershipValuesSkill[vidas_restantes_pocas.name] = vidas_restantes_pocas.eval(crispInputVidasRestantes);
	inputMembershipValuesSkill[vidas_restantes_promedio.name] = vidas_restantes_promedio.eval(crispInputVidasRestantes);
	inputMembershipValuesSkill[vidas_restantes_muchas.name] = vidas_restantes_muchas.eval(crispInputVidasRestantes);

	//Tiempo de Vida
	inputMembershipValuesSkill[tiempo_de_vida_bajo.name] = tiempo_de_vida_bajo.eval(crispInputTiempoVida);
	inputMembershipValuesSkill[tiempo_de_vida_promedio.name] = tiempo_de_vida_promedio.eval(crispInputTiempoVida);
	inputMembershipValuesSkill[tiempo_de_vida_alto.name] = tiempo_de_vida_alto.eval(crispInputTiempoVida);

	//Score
	inputMembershipValuesSkill[score_bajo.name] = score_bajo.eval(crispInputScore);
	inputMembershipValuesSkill[score_promedio.name] = score_promedio.eval(crispInputScore);
	inputMembershipValuesSkill[score_alto.name] = score_alto.eval(crispInputScore);


	//Show fuzzy membership values 
	for(map<string, double>::iterator itr = inputMembershipValuesSkill.begin(); itr != inputMembershipValuesSkill.end(); ++itr)
	{
		cout << (*itr).first << " -> " << (*itr).second << endl;
	}
 
	// Rules 
	Rules rulesSkill;

	rulesSkill.addRule("IF Tiempo_De_Vida_Bajo OR Vidas_Restantes_Pocas OR Score_Bajo THEN Skill_Jugador_Bajo");
	rulesSkill.addRule("IF Tiempo_De_Vida_Promedio OR Vidas_Restantes_Promedio OR Score_Promedio THEN Skill_Jugador_Promedio");
	rulesSkill.addRule("IF Tiempo_De_Vida_Alto OR Vidas_Restantes_Muchas OR Score_Alto THEN Skill_Jugador_Alto");

	cout << "Rules added" << endl;

	map<string, double> outputValuesSkill = rulesSkill.inferMamdani(inputMembershipValuesSkill);

	cout << endl;
	cout << "Output values are: " << endl;

	for(map<string, double>::iterator itr = outputValuesSkill.begin(); itr != outputValuesSkill.end(); ++itr)
	{
		cout << (*itr).first << " -> " << (*itr).second << endl;
	}

	cout << endl;
	cout << endl;

	/*************************** Controlador para los comportamientos ***************************/

	double TAMAÑO_PANTALLA = 400;
	double TIEMPO_ESTRELLA = 9.0;

	// Crisp Inputs 
	double crispInputDistancia = 25;
	double crispInputTiempoEstrella = 7.9; //Mario lleva este tiempo con la estrella

	// Input Fuzzy Sets 
	FuzzySet distancia_cerca("Distancia_Cerca");
	FuzzySet distancia_media("Distancia_Media");
	FuzzySet distancia_lejos("Distancia_Lejos");

	FuzzySet tiempo_estrella_poco("Tiempo_Estrella_Poco");
	FuzzySet tiempo_estrella_mucho("Tiempo_Estrella_Mucho");


	// Output Fuzzy Sets 
	FuzzySet comportamiento_atacar("Comportamiento_Atacar");
	FuzzySet comportamiento_atacar_inteligentemente("Comportamiento_Atacar_Inteligentemente");
	FuzzySet comportamiento_huir("Comportamiento_Huir");
	FuzzySet comportamiento_patrullar("Comportamiento_Patrullar");


	// Membership Functions 

	//Distancia
	double distancia_cerca_args[] = {0, TAMAÑO_PANTALLA / 4};
	distancia_cerca.setMF(SAT, distancia_cerca_args, 2);
	
	double distancia_media_args[] = {0, TAMAÑO_PANTALLA / 2, TAMAÑO_PANTALLA};
	distancia_media.setMF(TRIANG, distancia_media_args, 3);

	double distancia_lejos_args[] = {TAMAÑO_PANTALLA, TAMAÑO_PANTALLA / 2};
	distancia_lejos.setMF(SAT, distancia_lejos_args, 2);

	//Tiempo Estrella -> "Queda poco/mucho tiempo para que se termine el efecto de la estrella"
	//Primera amplitud era de 0.9, pero funciona mejor con 2.25
	double tiempo_estrella_poco_args[] = {TIEMPO_ESTRELLA, 2.25};
	tiempo_estrella_poco.setMF(GAUSS, tiempo_estrella_poco_args, 2);

	double tiempo_estrella_mucho_args[] = {0, 3.2};
	tiempo_estrella_mucho.setMF(GAUSS, tiempo_estrella_mucho_args, 2);

	cout << "Fuzzification" << endl;

	//Fuzzification
	map<string, double> inputMembershipValuesBehavior;

	//Distancia
	inputMembershipValuesBehavior[distancia_cerca.name] = distancia_cerca.eval(crispInputDistancia);
	inputMembershipValuesBehavior[distancia_media.name] = distancia_media.eval(crispInputDistancia);
	inputMembershipValuesBehavior[distancia_lejos.name] = distancia_lejos.eval(crispInputDistancia);

	//Tiempo Estrella
	inputMembershipValuesBehavior[tiempo_estrella_poco.name] = tiempo_estrella_poco.eval(crispInputTiempoEstrella);
	inputMembershipValuesBehavior[tiempo_estrella_mucho.name] = tiempo_estrella_mucho.eval(crispInputTiempoEstrella);

	//Skill Jugador (utilizar los valores calculados por el controlador anterior)
	inputMembershipValuesBehavior[skill_jugador_bajo.name] = outputValuesSkill.find(skill_jugador_bajo.name) -> second;
	inputMembershipValuesBehavior[skill_jugador_promedio.name] = outputValuesSkill.find(skill_jugador_promedio.name) -> second;
	inputMembershipValuesBehavior[skill_jugador_alto.name] = outputValuesSkill.find(skill_jugador_alto.name) -> second;

	//Show fuzzy membership values 
	for(map<string, double>::iterator itr = inputMembershipValuesBehavior.begin(); itr != inputMembershipValuesBehavior.end(); ++itr)
	{
		cout << (*itr).first << " -> " << (*itr).second << endl;
	}

	// Rules 
	Rules rulesBehavior;

	//Atacar
	//rulesBehavior.addRule("IF Distancia_Lejos OR Distancia_Media OR Distancia_Cerca AND Tiempo_Estrella_Poco AND Skill_Jugador_Alto THEN Comportamiento_Atacar_Inteligentemente");
	rulesBehavior.addRule("IF Tiempo_Estrella_Poco AND Skill_Jugador_Alto THEN Comportamiento_Atacar_Inteligentemente");
	rulesBehavior.addRule("IF Distancia_Cerca AND Skill_Jugador_Promedio AND Tiempo_Estrella_Poco THEN Comportamiento_Atacar");

	//Huir
	rulesBehavior.addRule("IF Skill_Jugador_Bajo AND Distancia_Cerca THEN Comportamiento_Huir");
	rulesBehavior.addRule("IF Distancia_Cerca OR Distancia_Media AND Tiempo_Estrella_Mucho THEN Comportamiento_Huir");

	//Patrullar
	rulesBehavior.addRule("IF Skill_Jugador_Bajo OR Skill_Jugador_Promedio AND Distancia_Media THEN Comportamiento_Patrullar");
	rulesBehavior.addRule("IF Skill_Jugador_Bajo OR Skill_Jugador_Promedio AND Distancia_Lejos THEN Comportamiento_Patrullar");
	

	cout << "Rules added" << endl;

	map<string, double> outputValuesBehavior = rulesBehavior.inferMamdani(inputMembershipValuesBehavior);

	cout << endl;
	cout << "Output values are: " << endl;

	for(map<string, double>::iterator itr = outputValuesBehavior.begin(); itr != outputValuesBehavior.end(); ++itr)
	{
		cout << (*itr).first << " -> " << (*itr).second << endl;
	}
}

/******* Membership Functions *******/
double triangmf(double left, double center, double right, double x)
{
	double res = 0;

	if(x <= left)							res = 0;
	else if(x >= right)						res = 0;
	else if(x > left && x <= center)		res = (x - left) / (center - left);
	else if(x < right && x > center)		res = 1 - abs((right - x) / (center - right));


	return res;
}

double trapmf(double lowLeft, double upLeft, double upRight, double lowRight, double x)
{
	double res = 0;

	if(x <= lowLeft)						res = 0;
	else if(x >= lowRight)					res = 0;
	else if(x > lowLeft && x <= upLeft)		res = (x - lowLeft) / (upLeft - lowLeft);
	else if(x > upLeft && x <= upRight)		res = 1;
	else if(x > upRight && x < lowRight)	res = 1 - abs((upRight - x) / (lowRight - upRight));

	return res;
}

double satmf(double up, double down, double x)
{
	double res = 0;

	//Left saturation
	if(up < down)
	{
		if(x <= up)				res = 1;
		else if(x >= down)		res = 0;
		else 					res = 1 - abs((up - x) / (down - up));
	}
	//Right saturation
	else
	{
		if(x >= up)				res = 1;
		else if(x <= down)		res = 0;
		else 					res = (x - down) / (up - down);
	}

	return res;
}

double gaussianmf(double center, double width, double x)
{
	return exp(-pow((x - center) / (sqrt(2 * width)), 2));
}



/******* Norms *******/
//Vector version
double fAnd(vector<double> &args)
{
	int minimum = args.front();

	for(int i = 0; i < args.size(); i++)
	{
		if(minimum > args[i])	minimum = args[i];
	}

	return minimum;
}

//Two arguments version
double fAnd(double a, double b)
{
	return min(a, b);
}

//Vector version
double fOr(vector<double> &args)
{
	int maximum = args.front();

	for(int i = 0; i < args.size(); i++)
	{
		if(maximum < args[i])	maximum = args[i];
	}

	return maximum;
}

//Two arguments version
double fOr(double a, double b)
{
	return max(a, b);
}