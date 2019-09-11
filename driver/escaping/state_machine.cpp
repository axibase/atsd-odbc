#include "state_machine.h"

BaseState* SelectState::nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(matchesColumnRegex(literal)) {
                return (BaseState*) new ColumnState(nextToken, stateMachine);
            } else {
                return (BaseState*) new FunctionOrClauseState(nextToken, stateMachine); 
            }
        };

BaseState* ColumnState::nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(inTableKeywordsList(literal)) {
                return (BaseState*) new TableState(nextToken, stateMachine);
            } else if (literal == "AS") {
                return (BaseState*) new IntermediateState(nextToken, stateMachine, this);
            } else if(literal == ",") {
                nextIsColumnOrFunction = true; //if next token is ",", then next state cannot be allies in select
                return (BaseState*) new IntermediateState(nextToken, stateMachine, this);
            } else if (matchesColumnRegex(literal)) {
                if(!nextIsColumnOrFunction || stateMachine.isInAlliesList(literal)) {
                    return (BaseState*) new AlliesState(nextToken, stateMachine);
                } else {
                    return (BaseState*) new ColumnState(nextToken, stateMachine);
                }
            } else {
                return (BaseState*) new FunctionOrClauseState(nextToken, stateMachine); 
            }
        }
std::string ColumnState::convert() {
            std::string literal = token.literal.to_string();
            std::smatch match;
            if (std::regex_search(literal, match, COLUMN_WITH_POINT_REGEX)) {
					std::string prefix = match.prefix().str(); //table name with "." symbol
					std::string column = literal.substr(prefix.size(), literal.size() - prefix.size());
					size_t delimeter_pos = column.find(".");
					column.replace(delimeter_pos, 1, "\".\"");
					return prefix + column;
				} else {
					return literal;
				}
        }

BaseState* AlliesState::nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(literal == ",") {
                return (BaseState*) new IntermediateState(nextToken, stateMachine, this);
            } else if(stateMachine.isInAlliesList(literal)) {
                return (BaseState*) new AlliesState(nextToken, stateMachine);
            } else if(inTableKeywordsList(literal)) {
                return (BaseState*) new TableState(nextToken, stateMachine);
            } else if(matchesColumnRegex(literal)) {
                return (BaseState*) new ColumnState(nextToken, stateMachine);
            } else {
                return (BaseState*) new FunctionOrClauseState(nextToken, stateMachine);
            }
        }


AlliesState::AlliesState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {
            if(!stateMachine.isInAlliesList(token.literal.to_string())) {
                stateMachine.addAllies(token.literal.to_string());
            }
        };

BaseState* FunctionOrClauseState::nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(literal == ",") {
                return (BaseState*) new IntermediateState(nextToken, stateMachine, this);
            } else if(stateMachine.isInAlliesList(literal)) {
                return (BaseState*) new AlliesState(nextToken, stateMachine);
            } else if(inTableKeywordsList(literal)) {
                return (BaseState*) new TableState(nextToken, stateMachine);
            } else if(matchesColumnRegex(literal)) {
                return (BaseState*) new ColumnState(nextToken, stateMachine);
            } else {
                return (BaseState*) new FunctionOrClauseState(nextToken, stateMachine);
            }
        }

BaseState* TableState::nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(literal == "AS" || matchesColumnRegex(literal)) {
                return (BaseState*) new TableState(nextToken, stateMachine);
            } else {
                return (BaseState*) new FunctionOrClauseState(nextToken, stateMachine);
            }
        }

std::string StateMachine::run() {
            std::string modified_query;
            Token current_token(lex->Consume());
            if(to_upper(current_token.literal) != "SELECT") {
                return queryView.to_string();
            }
            currentState = (BaseState*) new SelectState(current_token, *this);
            bool read_from_state = true;
            for( ; (current_token.type != Token::EOS) && (level >= 0); current_token = lex->Consume()) {
                if(read_from_state) {
                    modified_query += currentState->convert();
                } else if(current_token.type == Token::LPARENT){
                    level++;
                    modified_query += "(";
                } else if(current_token.type == Token::RPARENT) {
                    level--;
                    modified_query += ")";
                } else {
                    modified_query += current_token.literal.to_string();
                }

                modified_query += " ";
                const Token next_token = lex->LookAhead(1);
                read_from_state = false;
                if(to_upper(next_token.literal) == "SELECT") {
                    StateMachine subqueryMachine(lex);
                    modified_query += subqueryMachine.run();
                    level--;
                } else if((next_token.type == Token::IDENT) || (next_token.type == Token::COMMA) || (std::find(function_list.begin(), function_list.end(), next_token.type) != function_list.end())) {
                    BaseState* next_state = currentState->nextState(next_token);
                    delete currentState;
                    currentState = next_state;
                    read_from_state = true;
                }
            }
        };