/*
 * Public Domain (www.unlicense.org)
 *
 * This is free and unencumbered software released into the public domain.
 * Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
 * software, either in source code form or as a compiled binary, for any purpose,
 * commercial or non-commercial, and by any means.
 * In jurisdictions that recognize copyright laws, the author or authors of this
 * software dedicate any and all copyright interest in the software to the public
 * domain. We make this dedication for the benefit of the public at large and to
 * the detriment of our heirs and successors. We intend this dedication to be an
 * overt act of relinquishment in perpetuity of all present and future rights to
 * this software under copyright law.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <vector>
#include <cstdio>
#include <cstdlib>


bool cstrings_equal(char * first, char *second) {
	while (*first && *second) {
		if (*first != *second) {
			return false;
		}
		first++;
		second++;
	}

	return *first == *second;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tokenizer
//////////////////////////////////////////////////////////////////////////////////////////////////////////
enum TokenKind {
	TokenKind_Variable,
	TokenKind_Plus,
	TokenKind_Minus,
	TokenKind_Slash,
	TokenKind_Asterisk,
	TokenKind_LessThan,
	TokenKind_GreaterThan,
	TokenKind_EOF,
	TokenKind__COUNT
};

struct Token {
	TokenKind kind;
	char variable_name;
};

#define LETTER 'a': \
	case 'b': \
	case 'c': \
	case 'd': \
	case 'e': \
	case 'f': \
	case 'g': \
	case 'h': \
	case 'i': \
	case 'j': \
	case 'k': \
	case 'l': \
	case 'm': \
	case 'n': \
	case 'o': \
	case 'p': \
	case 'q': \
	case 'r': \
	case 's': \
	case 't': \
	case 'u': \
	case 'v': \
	case 'w': \
	case 'x': \
	case 'y': \
	case 'z'

static std::vector<Token> tokenize(char * source) {
	std::vector<Token> tokens;
	bool keep_looping = true;

	while (keep_looping) {
		char ch = *source;
		source++;
		switch (ch) {
			case '+': tokens.push_back({ TokenKind_Plus }); break;
			case '-': tokens.push_back({ TokenKind_Minus }); break;
			case '/': tokens.push_back({ TokenKind_Slash }); break;
			case '*': tokens.push_back({ TokenKind_Asterisk }); break;
			case '<': tokens.push_back({ TokenKind_LessThan }); break;
			case '>': tokens.push_back({ TokenKind_GreaterThan }); break;
			case LETTER: tokens.push_back({ TokenKind_Variable, ch }); break;
			case 0:
				  keep_looping = false;
				  break;
			default: break;
		}
	}
	return tokens;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parser (common stuff)
//////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Parser {
	std::vector<Token> tokens;
	std::size_t current_token_offset;
};


enum NodeKind {
	NodeKind_Add,
	NodeKind_Sub,
	NodeKind_Div,
	NodeKind_Mul,
	NodeKind_Variable,
	NodeKind_CompareLessThan,
	NodeKind_CompareGreaterThan,
	NodeKind__Invalid
};

struct Node {
	NodeKind kind;
	union {
		struct {
			Node * left;
			Node * right;
		};
		char variable_name;
	};
};

static Node * make_variable_name_node(char variable_name) {
	Node * node = new Node;
	node->kind = NodeKind_Variable;
	node->left = node->right = nullptr;
	node->variable_name = variable_name;
	return node;
}


static Node * make_binary_op_node(NodeKind op, Node * left, Node * right) {
	Node * node = new Node;
	node->kind = op;
	node->left = left;
	node->right = right;
	return node;
}

static NodeKind to_binary_op(Token token) {
	if (token.kind == TokenKind_Plus) return NodeKind_Add;
	if (token.kind == TokenKind_Minus) return NodeKind_Sub;
	if (token.kind == TokenKind_Asterisk) return NodeKind_Mul;
	if (token.kind == TokenKind_Slash) return NodeKind_Div;
	if (token.kind == TokenKind_LessThan) return NodeKind_CompareLessThan;
	if (token.kind == TokenKind_GreaterThan) return NodeKind_CompareGreaterThan;

	return NodeKind__Invalid;
}

static bool is_binary_operator(Token token) {
	if (token.kind == TokenKind_Plus) return true;
	if (token.kind == TokenKind_Minus) return true;
	if (token.kind == TokenKind_Slash) return true;
	if (token.kind == TokenKind_Asterisk) return true;
	if (token.kind == TokenKind_LessThan) return true;
	if (token.kind == TokenKind_GreaterThan) return true;

	return false;
}

static Token get_next_token(Parser * parser) {
	if (parser->tokens.size() == parser->current_token_offset) {
		return { TokenKind_EOF };
	}

	return parser->tokens[parser->current_token_offset++];
}

static Node * parse_leaf(Parser * parser) {
	Token next_token = get_next_token(parser);

	if (next_token.kind != TokenKind_Variable) return nullptr;

	return make_variable_name_node(next_token.variable_name);
}

static void go_back_to_previous_token(Parser * parser) {
	parser->current_token_offset -= 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Naive parsing
//////////////////////////////////////////////////////////////////////////////////////////////////////////
static Node * parse_expression_naive(Parser * parser) {
	Node * left = parse_leaf(parser);

	Token next_token = get_next_token(parser);
	if (is_binary_operator(next_token)) {
		Node * right = parse_expression_naive(parser);
	
		return make_binary_op_node(to_binary_op(next_token), left, right);

	}

	return left;
}

static Node * parse_naive(char * source) {
	Parser p = {
		.tokens = tokenize(source),
		.current_token_offset = 0,
	};

	return parse_expression_naive(&p);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parsing with tree rewriting
//////////////////////////////////////////////////////////////////////////////////////////////////////////
static void print_node(Node * node);

static Node * parse_expression_tree_rewriting_complete(Parser * parser) {
	Node * left = parse_leaf(parser);

	Token next_token = get_next_token(parser);

	if (is_binary_operator(next_token)) {
		Node * right = parse_expression_tree_rewriting_complete(parser);
		Node * current_node = make_binary_op_node(to_binary_op(next_token), left, right);

		Node dummy = { .left = current_node };
		Node * current_node_parent = &dummy;

		if (current_node->kind == NodeKind_Mul || current_node->kind == NodeKind_Div) {
			while (current_node->right->kind == NodeKind_Mul ||
			       current_node->right->kind == NodeKind_Div ||
			       current_node->right->kind == NodeKind_Add ||
			       current_node->right->kind == NodeKind_Sub) {
	
				// Push the current node down the tree.
				Node * tmp = current_node->right;
				current_node->right = current_node->right->left;
				tmp->left = current_node;

				// Update the parent pointer.
				current_node_parent->left = tmp;

				// Set up the new parent.
				current_node_parent = tmp;
			}
		} else if (current_node->kind == NodeKind_Add || current_node->kind == NodeKind_Sub) {
			while (current_node->right->kind == NodeKind_Add ||
			       current_node->right->kind == NodeKind_Sub) {
	
				// Push the current node down the tree.
				Node * tmp = current_node->right;
				current_node->right = current_node->right->left;
				tmp->left = current_node;

				// Update the parent pointer.
				current_node_parent->left = tmp;

				// Set up the new parent.
				current_node_parent = tmp;
			}
		}

		return dummy.left;
	}

	return left;
}

static Node * parse_tree_rewriting_complete(char * source) {
	Parser p = {
		.tokens = tokenize(source),
		.current_token_offset = 0,
	};

	return parse_expression_tree_rewriting_complete(&p);
}

static Node * parse_expression_tree_rewriting(Parser * parser) {
	Node * left = parse_leaf(parser);

	Token next_token = get_next_token(parser);
	if (is_binary_operator(next_token)) {
		Node * right = parse_expression_tree_rewriting(parser);
		Node * result = make_binary_op_node(to_binary_op(next_token), left, right);

		if (result->kind == NodeKind_Mul || result->kind == NodeKind_Div) {
			if (result->right->kind == NodeKind_Add || result->right->kind == NodeKind_Sub) {
				result->right = right->left;
				right->left = result;
				result = right;
			}
		}
		return result;
	}

	return left;
}

static Node * parse_tree_rewriting(char * source) {
	Parser p = {
		.tokens = tokenize(source),
		.current_token_offset = 0,
	};

	return parse_expression_tree_rewriting(&p);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Operator precedence parsing
//////////////////////////////////////////////////////////////////////////////////////////////////////////
static int PRECEDENCE_TABLE[TokenKind__COUNT] = {
	0,  // TokenKind_Variable
	2,  // TokenKind_Plus
	2,  // TokenKind_Minus
	3,  // TokenKind_Slash
	3,  // TokenKind_Asterisk
	1,  // TokenKind_LessThan
	1,  // TokenKind_GreaterThan
	0, // TokenKind_EOF
};

static Node * parse_expression(Parser * parser, int min_precedence);

static Node * parse_increasing_precedence(Parser * parser, Node * left, int min_precedence) {
	Token next_token = get_next_token(parser);
	int precedence = PRECEDENCE_TABLE[next_token.kind];
	if (precedence <= min_precedence) {
		go_back_to_previous_token(parser);
		return left;
	}

	if (is_binary_operator(next_token)) {
		Node * right = parse_expression(parser, precedence);
		return make_binary_op_node(to_binary_op(next_token), left, right);
	} else {
		return left;
	}
}

static Node * parse_expression(Parser * parser, int min_precedence) {
	Node * left = parse_leaf(parser);

	while (true) {
		Node * node = parse_increasing_precedence(parser, left, min_precedence);

		if (node == left) break;
		left = node;
	};

	return left;
}

static Node * parse_pratt(char * source) {
	Parser p = {
		.tokens = tokenize(source),
		.current_token_offset = 0
	};

	return parse_expression(&p, -1);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
//////////////////////////////////////////////////////////////////////////////////////////////////////////
static void print_node2(Node * node) {
	if (!node) return;

	if (node->kind == NodeKind_Variable) printf("%c", node->variable_name);
	if (node->kind == NodeKind_CompareLessThan) {
		printf("( < ");
		print_node2(node->left);
		printf(" ");
		print_node2(node->right);
		printf(" )");
	}
	if (node->kind == NodeKind_CompareGreaterThan) {
		printf("( > ");
		print_node2(node->left);
		printf(" ");
		print_node2(node->right);
		printf(" )");
	}
	if (node->kind == NodeKind_Add) {
		printf("( + ");
		print_node2(node->left);
		printf(" ");
		print_node2(node->right);
		printf(" )");
	}
	if (node->kind == NodeKind_Sub) {
		printf("( - ");
		print_node2(node->left);
		printf(" ");
		print_node2(node->right);
		printf(" )");
	}
	if (node->kind == NodeKind_Mul) {
		printf("( * ");
		print_node2(node->left);
		printf(" ");
		print_node2(node->right);
		printf(" )");
	}
	if (node->kind == NodeKind_Div) {
		printf("( / ");
		print_node2(node->left);
		printf(" ");
		print_node2(node->right);
		printf(" )");
	}

}

static void print_node(Node * node) {
	print_node2(node);
	putchar('\n');
}

#define ARRAY_SIZE(ARRAY) (sizeof((ARRAY)) / sizeof((ARRAY)[0]))


static void print_usage(char * program_name) {
	printf("Usage: %s METHOD\n\n"
			"Options:\n"
			"\tMETHOD\tone of naive, tree-rewriting, tree-rewriting-complete, or pratt\n\n", program_name);
}

typedef Node * (ParseFn)(char *);

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Bad arguments\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	struct ParseMethod{
		char * name;
		ParseFn * parse_fn;
	} methods[] = {
		{"naive", &parse_naive},
		{"tree-rewriting", &parse_tree_rewriting},
		{"tree-rewriting-complete", &parse_tree_rewriting_complete},
		{"pratt", &parse_pratt},
	};

	ParseMethod selected_method = {};
	for (std::size_t i = 0; i < ARRAY_SIZE(methods); i++) {
		if (cstrings_equal(methods[i].name, argv[1])) {
			selected_method = methods[i];
			break;
		}
	}

	if (!selected_method.name) {
		fprintf(stderr, "Invalid method: %s", argv[1]);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	char * test_cases[] = {
		"a + b + c + d",
		"a - b + c",
		"a + b * c + d",
		"a / b - c",
		"a / b * c",
		"a / b * c + d",
		"a * b + c + d",
		"a * b * c * d",
	};

	printf("--- Method: %s\n", selected_method.name);
	for (std::size_t i = 1; i <= ARRAY_SIZE(test_cases); i++) {
		printf("=== Test #%02ld: %s\n", i, test_cases[i-1]);
		print_node(selected_method.parse_fn(test_cases[i-1]));
		printf("\n\n");
	}

	return 0;
}
