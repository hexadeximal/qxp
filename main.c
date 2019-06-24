#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

struct node {
    char *q;
    struct node *next;
    struct node *prev;
};



enum Error {	
	ER_ALLOC = -1,
	ER_XLOADFILE = -2
};

xmlDoc *xml_load_doc(const char *fname)
{
	xmlDoc *doc = xmlParseFile(fname);

	if(doc == NULL) {
		fprintf(stderr, "document not parsed successfully\n");
		xmlFreeDoc(doc);

		return NULL;
	}

	return doc;
}

void check_param(int argc, char **argv, char **file)
{
	if(argc < 2) {
		fprintf(stderr, "./qxp file.xml\n");
		exit(0);
	}

	(*file) = calloc(strlen(argv[1]) + 1, sizeof(char));

	if((*file) == NULL) {
		fprintf(stderr, "error alloc: %s %d\n", __FILE__, __LINE__);
		exit(ER_ALLOC);
	}

	strcpy((*file), argv[1]);
}

int read_command(char **cmd)
{
	int start_length = 512;
	
	(*cmd) = calloc(start_length, sizeof(char));
	
	if((*cmd) == NULL) {
		fprintf(stderr, "error alloc %s %d\n", __FILE__, __LINE__);
		return 1;
	}
	
	printf(": ");

	fgets((*cmd), start_length, stdin);
	
	return 0;
}

xmlXPathObject *get_nodeset(xmlDoc *doc, xmlChar *xpath)
{
	xmlXPathContext *context = NULL;
	xmlXPathObject *result = NULL;
	
	context = xmlXPathNewContext(doc);
	
	if(context == NULL) {
		fprintf(stderr, "error in context\n");
		return NULL;
	}
	
	result = xmlXPathEvalExpression(xpath, context);
	
	xmlXPathFreeContext(context);
	
	if(result == NULL) {
		xmlXPathFreeObject(result);
		printf("no result\n");
		return NULL;
	}
	
	return result;
}

int eval_xpath(xmlDoc *doc, const char *xpr)
{
	xmlXPathObject *result = NULL;
	result = get_nodeset(doc, (xmlChar *)xpr);
	
	if(result) {
		xmlNodeSet *nodeset;
		
		nodeset = result->nodesetval;
		/* if nr of results is negative, return */
		if(nodeset == NULL) {
			fprintf(stderr, "no results with query: %s\n", xpr);
			xmlXPathFreeObject(result);
			return -1;
		}
		
		if(nodeset->nodeNr == 0) {
			xmlXPathFreeObject(result);	
			return -1;
		}
		
		for(int i = 0; i < nodeset->nodeNr; i++) {
			xmlChar *tmp = NULL;
			xmlChar *t_name = NULL;
			
			tmp = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1); 
			t_name = xmlStrdup(nodeset->nodeTab[i]->name);
			
            printf("%s %s\n", t_name, tmp);
         	
			xmlFree(tmp);
			xmlFree(t_name);
		
			
		}
		
		xmlXPathFreeObject(result);
	}
	
	return 0;
}

void free_results(char ***result, char ***name, int *count)
{
	for(int i = 0; i < (*count); i++) {
		free((*result)[i]);
		free((*name)[i]);
	}
	
	free((*name));
	free((*result));
}

void print_results(char ***result, char ***name, int *count)
{
	for(int i = 0; i < (*count); i++) {
		printf("%s - %s\n", (*name)[i],(*result)[i]);
	}

}

void print_history(struct node *head)
{
    struct node *current = head;
    
    while(current != NULL) {
        printf("%s\n", current->q);
        current = current->next;
    }
}

void free_history(struct node *head)
{
    struct node *current = NULL;
    
    while(current != NULL) {
        current = head;
        head = head->next;
        free(current->q);
        free(current);
        
    }    
}

void push(struct node *head, const char *value)
{
    struct node *current = head;
    
    while(current->next != NULL) {
        current = current->next;
    }
    
    current->next = malloc(sizeof(struct node));
    
    if(current->next == NULL) {
        fprintf(stderr, "error alloc %s %d\n", __FILE__,__LINE__);
        return;
    }
    
    current->next->q = calloc(strlen(value) + 1, sizeof(char));
    
    if(current->next->q == NULL) {
        fprintf(stderr, "error alloc %s %d\n", __FILE__,__LINE__);
        return;        
    }
    
    strcpy(current->next->q, value);
    
    current->next->next = NULL;
}

int main(int argc, char **argv)
{
	char *file = NULL;
	xmlDoc *doc = NULL;
	char *query = NULL;
	bool running = false;
	struct node *history = NULL;
	check_param(argc, argv, &file);

    history = malloc(sizeof(struct node));
    
    if(history == NULL) {
        fprintf(stderr, "error alloc %s %d\n", __FILE__,__LINE__);
        return -1;
    }
    
    
	doc = xml_load_doc(file);

	if(doc != NULL) {
		printf("document: %s loaded\n", file);
		running = true;
	}

	while(running == true) {
		read_command(&query);

		if((strcmp(query, "exit\n") == 0) || strcmp(query, "q\n") == 0) {
			printf("cleanup\n");
			running = false;
		}
		if(strcmp(query, "history\n") == 0) {
            print_history(history);
        }

		eval_xpath(doc, query);

        
        if(history->q != NULL) {
            push(history, query);  
        }
        else {
         history->q = calloc(strlen(query) + 1, sizeof(char));
         
            if(history->q == NULL) {
                fprintf(stderr, "error alloc %s %d\n", __FILE__, __LINE__);
                return -1;
            }
            
            strcpy(history->q, query);
        }
        

        free(query);
	}


	free_history(history);
	
	free(file);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return 0;
}
