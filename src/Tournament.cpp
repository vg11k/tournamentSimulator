#include "Tournament.hpp"

using namespace std;
using Json = nlohmann::json;


Tournament::Tournament()
{
    //load all profiles
    m_profileManager = ProfileManager::getMe();



    ifstream inputFile("rosters/roster1.json");
    Json jsonObject;
    inputFile >> jsonObject;
    inputFile.close();


    int nbPersonnages = jsonObject[Constants::STRING_NAME_PERSONNAGES].size();
    m_personnages = new vector<Personnage>();
    for(int i = 0; i < nbPersonnages; i++)
    {
        Json jPersonnage  = jsonObject[Constants::STRING_NAME_PERSONNAGES][i];
        Personnage p = jPersonnage;
        if(p.areYouValid())
        {
            m_personnages->push_back(p);
        }
    }

    m_CCvsCCgrid = CSVParser::getCombatGridFromCSV("CCvsCC.csv");
    m_FvsEgrid = CSVParser::getCombatGridFromCSV("FvsE.csv");

}

int Tournament::getNew6D()
{
    return rand() %(6) + 1;
}

/*Duelliste * duelliste = p.getDuelliste();
       duelliste->getPersonnage()->sePresenter();
       delete duelliste;*/

Tournament::~Tournament()
{
    //dtor
}


bool Tournament::aTouche(int roll, int CCa, int CCc)
{
    return m_CCvsCCgrid->at(CCc + 1)->at(CCa + 1) <= roll;
}

bool Tournament::aBlesse(int roll, int force, int endu)
{
    vector<int> * line = m_FvsEgrid->at(endu + 1);
    int val = line->at(force + 1);
    return val <= roll;
}

void Tournament::makeFight(Duelliste * d1, Duelliste * d2)
{

    bool sameInit = d1->getInitiative() == d2->getInitiative();
    bool d1AttaqueEnPremier;
    if(d1->getInitiative() > d2->getInitiative())
    {
        d1AttaqueEnPremier = true;
    }
    else
    {
        d1AttaqueEnPremier = false;
    }

    bool stop = false;

    while(!stop)
    {
        Duelliste * premierAAttaquer;
        Duelliste * secondAAttaquer;

        if(d1AttaqueEnPremier)
        {
            premierAAttaquer = d1;
            secondAAttaquer = d2;
        }
        else
        {
            premierAAttaquer = d2;
            secondAAttaquer = d1;
        }


        if(peutAttaquer(premierAAttaquer, secondAAttaquer))
        {
            effectuerAttaques(premierAAttaquer, secondAAttaquer);
        }

        if(sameInit || secondAAttaquer->getCurrentHP() > 0 )
        {
            if(peutAttaquer(secondAAttaquer, premierAAttaquer))
            {
                effectuerAttaques(secondAAttaquer, premierAAttaquer);
            }

        }

        //mort simultanee : tout le monde repart avec 1hp pour un nouveau round
        if(premierAAttaquer->getCurrentHP() < 1 && secondAAttaquer->getCurrentHP() < 1)
        {
            premierAAttaquer->setCurrentHP(1);
            secondAAttaquer->setCurrentHP(1);
        }

        if(premierAAttaquer->getCurrentHP() < 1 || secondAAttaquer->getCurrentHP() <1 )
        {
            stop = true;
            cout << "resultats : " << endl;
            cout << premierAAttaquer->getName() << " a " << premierAAttaquer->getCurrentHP() << " HP" << endl;
            cout << secondAAttaquer->getName() << " a " << secondAAttaquer->getCurrentHP() << " HP" << endl;
        }
    }
}


bool Tournament::peutAttaquer(Duelliste * attaquant, Duelliste * cible)
{
    return true;
}

void Tournament::effectuerAttaques(Duelliste * attaquant, Duelliste * cible)
{

    int CCdueliste1 = attaquant->getCapaciteCombat();
    int CCdueliste2 = cible->getCapaciteCombat();

    int forceDueliste1 = attaquant->getForce();
    int enduranceDueliste2 = cible->getEndurance();
    int attaquesDuelliste1 = attaquant->getAttaques();

    for(int attaqueIndex = 0; attaqueIndex < attaquesDuelliste1; attaqueIndex++)
    {
        int roll = getNew6D();
        cout << roll << " pour toucher " << endl;
        //reussir a attaquer
        if(aTouche(roll,CCdueliste1, CCdueliste2))
        {
            roll = getNew6D();
            cout << roll << " pour blesser " << endl;
            //reussir a blesser
            if(aBlesse(roll, forceDueliste1, enduranceDueliste2))
            {
                //passer les defenses
                if(!blessureAnnulee(attaquant, cible, false))
                {
                    //TODO ajouter BM
                    cible->removeHP(1);
                    cout << cible->getName() << " a perdu un pv." << endl;
                }
            }

        }
    }
}

bool Tournament::blessureAnnulee(Duelliste * attaquant, Duelliste * cible, bool attaqueSpeciale)
{

    int roll;

    //sauvegarde armure
    int modificateurDArmure = 0;
    if(attaquant->getForce() > 3)
    {
        modificateurDArmure += attaquant->getForce() - 3;
    }
    if(!attaqueSpeciale)
    {
        modificateurDArmure += attaquant->getPerforant();
    }

    int sauvegardeArmure = cible->getSauvegardeArmure() + modificateurDArmure;
    if(sauvegardeArmure < 7)
    {
        roll = getNew6D();
        if(roll >= sauvegardeArmure)
        {
            return true;
        }

    }

    //regen
    if(!attaquant->utiliseAttaquesEnflammees() && cible->beneficieRegeneration())
    {
        int valeurRegeneration = cible->getRegeneration();
        roll = getNew6D();
        if( roll >= valeurRegeneration)
        {
            return true;
        }
    }

    //invulnerable
    if(cible->beneficieInvulnerabilite())
    {
        int sauvegardeInvulnerable = cible->getSauvegardeInvulnerable();
        roll = getNew6D();
        if( roll >= sauvegardeInvulnerable)
        {
            if(!attaquant->utiliseAttaquesDivines())
            {
                return true;
            }
            else {
                roll = getNew6D();
                if( roll >= sauvegardeInvulnerable)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
