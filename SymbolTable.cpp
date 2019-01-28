//
//  SymbolTable.cpp
//  Optimiser
//
//  Created by Sharon John on 6/5/17.
//  Copyright © 2017 Sharon John. All rights reserved.
//

#include "SymbolTable.h"
#include <iostream>
#include <string>
#include <cctype>
#include <stack>
#include <vector>

using namespace std;

const int HASH_SIZE=19997;          //prime number chosen for increased efficiency of hash table
int scope=0;                        //scope variable keeps track of calls to enterScope() and exitScope()

struct Holder                       //Struct holding id and line number
{
    Holder(string id, int linenum)
    {
        m_id=id;
        m_linenum=linenum;
    }
    string m_id;
    int m_linenum;
};

//we will now create a vector of Holder objects to link to every bucket in the hash table


struct HashBuckets
{
    vector<Holder> HashVector;
};

//Hash table defined here featuring a key integer (hash function of string id) mapping on to a vector of type Holder where each element is an object that holds the id and line number

struct ScopeDepth
{
    vector<string> ID;
};

//This is a separate struct each object of which will hold a vector of strings containing the ID

class SymbolTableImpl
{
public:
    void enterScope();
    bool exitScope();
    bool declare(const string& id, int lineNum);
    int find(const string& id) const;
private:
    int HashFunc(const string& id) const;
    HashBuckets HashTable [HASH_SIZE];      //hash table with max buckets= 19,997
    stack<ScopeDepth> scopes;
    
    //stack  of type ScopeDepth keeps track of the scope every id is at. The depth of the stack is the total number of scopes created
};

int SymbolTableImpl::HashFunc(const string &id) const       //typical hash function for strings
{
    int total=0;
    for (int i=0; i<id.size(); i++)
    {
        total= total + (i+1)*id[i];
    }
    return total % HASH_SIZE;
}

void SymbolTableImpl::enterScope()
{
    scope++;
    ScopeDepth s;
    
    scopes.push(s);
}

//Since the scopes are maintained by the stack and not the hash table, we push every new object to the stack. We also increment the scope counter accordingly

bool SymbolTableImpl::exitScope()
{
    if (scopes.empty() || scope==1)
        return false;
   
    for (int i = 0; i < scopes.top().ID.size(); i++)
    {
        int value=HashFunc(scopes.top().ID[i]);
        HashTable[value].HashVector.pop_back();
    }
    scopes.pop();
    scope--;
    return true;
}

//exitScope checks first to see if the stack is empty or if the counter is 1, if not, it proceeds to pop the stack. Also need to loop through the vector at the top level of the stack, find the id of each element of the vector, hash it using the function and then look for the value in the Hash table, then pop the back of the HashVector as that is the most recent declaration. In this manner, we avoid having to actually loop through the HashVector, so we maximise efficiency.


bool SymbolTableImpl::declare(const string& id, int lineNum)
{
    int value=HashFunc(id);
    if (scopes.empty())
        enterScope();
    
    for (int i=0; i<scopes.top().ID.size(); i++)
    {
        if (scopes.top().ID.empty())
        {
            break;
        }
        
        if (id==scopes.top().ID[i])
            return false;
    }
    
    scopes.top().ID.push_back(id);
    
    Holder sub(id, lineNum);
    HashTable[value].HashVector.push_back(sub);
    
    return true;
};

//if stack is empty, enterScope. This is done to avoid bad access as we are then immediately looping through the stack itself.


int SymbolTableImpl::find(const string& id) const
{
    if (id.empty())
        return -1;
    
    int value=HashFunc(id);
    
    size_t i = HashTable[value].HashVector.size();
    while (i > 0)
    {
        i--;
        if (HashTable[value].HashVector[i].m_id == id)
            return HashTable[value].HashVector[i].m_linenum;
    }
    
    return -1;
};

//We ignore the stack completely for this as firstly, the stack does not hold the line numbers and secondly because, the hash table provides us with O(1) look up and it contains all the necessary information. Borrow the size_t from the original implementation as the lack of loss in precision seems to increase speed after testing.



//*********** SymbolTable functions **************

// For the most part, these functions simply delegate to SymbolTableImpl's
// functions.

SymbolTable::SymbolTable()
{
    m_impl = new SymbolTableImpl;
}

SymbolTable::~SymbolTable()
{
    delete m_impl;
}

void SymbolTable::enterScope()
{
    m_impl->enterScope();
}

bool SymbolTable::exitScope()
{
    return m_impl->exitScope();
}

bool SymbolTable::declare(const string& id, int lineNum)
{
    return m_impl->declare(id, lineNum);
}

int SymbolTable::find(const string& id) const
{
    return m_impl->find(id);
}
