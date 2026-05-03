#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- STRUCTURE ---------- */

struct Victim
{
    char id[10];
    char name[50];
    int age;
    int injury;
    char location[50];
    char shelterId[10];
};

struct Shelter
{
    char id[10];
    char name[50];
    char location[50];
    int capacity;
    int current;
};

struct Resource
{
    char id[10];
    char name[50];
    int quantity;
    char type[30]; // e.g., Food, Water, Medicine
};

struct Volunteer
{
    char id[10];
    char name[50];
    char role[30]; // Medical, Food, Transport, etc.
};

struct Distribution
{
    char victimId[10];
    char resourceId[10];
    int quantity;
};

/* ---------- FUNCTION PROTOTYPES ---------- */

/* Victim */
void victimMenu();
void addVictim();
void displayVictims();
void searchVictim();
void updateVictim();
void deleteVictim();
void sortVictims();

/* Shelter */
void shelterMenu();
void addShelter();
void displayShelters();
void searchShelter();
void updateShelter();
void deleteShelter();

/* Resource */
void resourceMenu();
void addResource();
void displayResources();
void searchResource();
void updateResource();
void deleteResource();

/* Volunteer */
void volunteerMenu();
void addVolunteer();
void displayVolunteers();
void searchVolunteer();
void updateVolunteer();
void deleteVolunteer();

/* Distribution */
void distributionMenu();
void assignResource();
void displayDistributions();

/* System Summary */
void systemSummary();

/* Input Validation */
int isValidRole(char role[]);
void normalizeID(char id[]);
void clearBuffer();

/* ---------- MAIN ---------- */
int main()
{
    int choice;

    while(1)
    {
        printf("\n===== DISASTER RELIEF MANAGEMENT SYSTEM =====\n");
        printf("1. Victim Management\n");
        printf("2. Shelter Management\n");
        printf("3. Resource Management\n");
        printf("4. Volunteer Management\n");
        printf("5. Distribution Management\n");
        printf("6. System Summary\n");
        printf("7. Exit\n");

        printf("Enter choice: ");
        if(scanf("%d", &choice) != 1)
        {
            printf("\nInvalid input!\n");
            while(getchar()!='\n');
            continue;
        }

        switch(choice)
        {
            case 1: victimMenu(); break;
            case 2: shelterMenu(); break;
            case 3: resourceMenu(); break;
            case 4: volunteerMenu(); break;
            case 5: distributionMenu(); break;
            case 6: systemSummary(); break;
            case 7: 
                printf("\nProgram Closed\n");
                exit(0);

            default:
                printf("\nInvalid choice\n");
        }
    }
}

/* ---------- VICTIM MODULE ---------- */

void victimMenu()
{
    int ch;

    while(1)
    {
        printf("\n--- Victim Menu ---\n");
        printf("1. Add Victim\n");
        printf("2. Display Victims\n");
        printf("3. Search Victim\n");
        printf("4. Update Victim\n");
        printf("5. Delete Victim\n");
        printf("6. Sort Victims by Priority\n");
        printf("7. Back\n");
        printf("8. Exit Program\n");

        printf("Enter choice: ");
        if(scanf("%d", &ch) != 1)
        {
            printf("\nInvalid input!\n");
            while(getchar()!='\n');
            continue;
        }

        switch(ch)
        {
            case 1: addVictim(); break;
            case 2: displayVictims(); break;
            case 3: searchVictim(); break;
            case 4: updateVictim(); break;
            case 5: deleteVictim(); break;
            case 6: sortVictims(); break;
            case 7: return;
            case 8:
                printf("\nProgram Closed\n");
                exit(0);
            default: printf("\nInvalid choice\n");
        }
    }
}

void addVictim()
{
    printf("\n--- Add Victim ---\n");
    FILE *fp = fopen("victim.dat", "ab");
    if(fp == NULL)
    {
        printf("\nFile error!\n");
        return;
    }

    struct Victim v;

    printf("Enter Victim ID: ");
    scanf("%s", v.id);
    normalizeID(v.id);

    FILE *check = fopen("victim.dat", "rb");
    struct Victim temp;

    if(check != NULL)
    {
        while(fread(&temp, sizeof(temp), 1, check))
        {
            if(strcmp(temp.id, v.id) == 0)
            {
                printf("\nID already exists!\n");
                fclose(check);
                fclose(fp);
                return;
            }
        }
        fclose(check);
    }

    clearBuffer();
    printf("Enter Name: ");
    fgets(v.name, sizeof(v.name), stdin);
    v.name[strcspn(v.name, "\n")] = 0;

    printf("Enter Age: ");
    if(scanf("%d",&v.age) != 1 || v.age <= 0)
    {
        printf("\nInvalid age!\n");
        while(getchar()!='\n');
        fclose(fp);
        return;
    }

    printf("Enter Injury Level: ");
    if(scanf("%d",&v.injury) != 1 || v.injury <= 0)
    {
        printf("\nInvalid injury level\n");
        while(getchar()!='\n');
        fclose(fp);
        return;
    }

    clearBuffer();
    printf("Enter Location: ");
    fgets(v.location, sizeof(v.location), stdin);
    v.location[strcspn(v.location, "\n")] = 0;
    normalizeID(v.location);

    //AUTO-ASSIGN

    FILE *sf = fopen("shelter.dat", "rb+");
    struct Shelter s;
    int assigned = 0;
    long pos;
    char selectedShelterId[10] = "NONE";

    if(sf != NULL)
    {
        /* PASS 1: SAME LOCATION */
        while(fread(&s, sizeof(s), 1, sf))
        {
            if(strcmp(s.location, v.location) == 0 &&
            s.current < s.capacity)
            {
                strcpy(selectedShelterId, s.id);
                pos = ftell(sf) - sizeof(s);
                assigned = 1;
                break;
            }
        }

        /* PASS 2: ANY AVAILABLE */
        if(!assigned)
        {
            rewind(sf);

            while(fread(&s, sizeof(s), 1, sf))
            {
                if(s.current < s.capacity)
                {
                    strcpy(selectedShelterId, s.id);
                    pos = ftell(sf) - sizeof(s);
                    assigned = 1;
                    break;
                }
            }
        }

        /* UPDATE SHELTER ONLY ONCE */
        if(assigned)
        {
            s.current++;

            fseek(sf, pos, SEEK_SET);
            fwrite(&s, sizeof(s), 1, sf);

            strcpy(v.shelterId, selectedShelterId);
        }

        fclose(sf);
    }

    /* If all shelters full */
    if(!assigned)
    {
        strcpy(v.shelterId, "NONE");
    }

    /* 1. Same location shelter
    2. Any available shelter
    3. NONE if all full */

    if(fwrite(&v, sizeof(v), 1, fp) == 1)
        printf("\nVictim added successfully!\n");
    else
        printf("\nWrite failed!\n");
    fclose(fp);
}

void displayVictims()
{
    printf("\n--- Display Victims ---\n");
    FILE *fp = fopen("victim.dat", "rb");
    if(fp == NULL)
    {
        printf("\nNo victim data found.\n");
        return;
    }

    struct Victim v;

    printf("\n--- Victim List ---\n");

    while(fread(&v, sizeof(v), 1, fp))
    {
        printf("\nID: %s\n", v.id);
        printf("Name: %s\n", v.name);
        printf("Age: %d\n", v.age);
        printf("Injury Level: %d\n", v.injury);
        printf("Location: %s\n", v.location);
        printf("Shelter ID: %s\n", v.shelterId);
    }

    fclose(fp);
}

void searchVictim()
{
    printf("\n--- Search Victim ---\n");
    FILE *fp = fopen("victim.dat", "rb");
    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Victim v;
    char id[10];
    int found = 0;

    printf("Enter Victim ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&v, sizeof(v), 1, fp))
    {
        if(strcmp(v.id, id) == 0)
        {
            printf("\nFound:\n");
            printf("Name: %s\n", v.name);
            printf("Age: %d\n", v.age);
            printf("Injury Level: %d\n", v.injury);
            printf("Location: %s\n", v.location);
            printf("Shelter: %s\n", v.shelterId);
            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nNot found!\n");

    fclose(fp);
}

void updateVictim()
{
    printf("\n--- Update Victim ---\n");
    FILE *fp = fopen("victim.dat", "rb+");

    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Victim v;
    char id[10];
    int found = 0;

    printf("Enter Victim ID: ");
    scanf("%9s", id);
    normalizeID(id);

    while(fread(&v, sizeof(v), 1, fp))
    {
        if(strcmp(v.id, id) == 0)
        {
            found = 1;

            char oldShelter[10];
            strcpy(oldShelter, v.shelterId);

            clearBuffer();

            printf("Enter new name: ");
            fgets(v.name,sizeof(v.name),stdin);
            v.name[strcspn(v.name,"\n")] = 0;

            printf("Enter new age: ");
            if(scanf("%d",&v.age) != 1 || v.age <= 0)
            {
                printf("\nInvalid age!\n");
                while(getchar()!='\n');
                fclose(fp);
                return;
            }

            printf("Enter new injury level: ");
            if(scanf("%d",&v.injury) != 1 || v.injury <= 0)
            {
                printf("\nInvalid injury level!\n");
                while(getchar()!='\n');
                fclose(fp);
                return;
            }

            clearBuffer();

            printf("Enter new location: ");
            fgets(v.location,sizeof(v.location),stdin);
            v.location[strcspn(v.location,"\n")] = 0;
            normalizeID(v.location);


            /* ---------- SAFE SHELTER REASSIGN ---------- */

            FILE *sf = fopen("shelter.dat","rb+");

            if(sf != NULL)
            {
                struct Shelter s;
                long pos;

                int assigned = 0;
                char newShelter[10];
                strcpy(newShelter, oldShelter); /* default keep same */

                /* Same location shelter first */
                while(fread(&s,sizeof(s),1,sf))
                {
                    if(strcmp(s.location,v.location)==0 &&
                       s.current < s.capacity)
                    {
                        strcpy(newShelter,s.id);
                        assigned = 1;
                        break;
                    }
                }

                /* Any available shelter */
                if(!assigned)
                {
                    rewind(sf);

                    while(fread(&s,sizeof(s),1,sf))
                    {
                        if(s.current < s.capacity)
                        {
                            strcpy(newShelter,s.id);
                            assigned = 1;
                            break;
                        }
                    }
                }

                if(assigned)
                {
                    /* Only adjust counts if shelter changed */
                    if(strcmp(oldShelter,newShelter)!=0)
                    {
                        /* Increase new shelter count */
                        rewind(sf);

                        while(fread(&s,sizeof(s),1,sf))
                        {
                            if(strcmp(s.id,newShelter)==0)
                            {
                                s.current++;

                                pos = ftell(sf)-sizeof(s);
                                fseek(sf,pos,SEEK_SET);
                                fwrite(&s,sizeof(s),1,sf);
                                break;
                            }
                        }

                        /* Decrease old shelter count */
                        rewind(sf);

                        while(fread(&s,sizeof(s),1,sf))
                        {
                            if(strcmp(s.id,oldShelter)==0)
                            {
                                if(s.current>0)
                                    s.current--;

                                pos = ftell(sf)-sizeof(s);
                                fseek(sf,pos,SEEK_SET);
                                fwrite(&s,sizeof(s),1,sf);
                                break;
                            }
                        }
                    }

                    strcpy(v.shelterId,newShelter);
                }
                else
                {
                    /* keep old shelter if no new one found */
                    strcpy(v.shelterId,oldShelter);
                }

                fclose(sf);
            }


            /* save updated victim */
            fseek(fp,-(long)sizeof(v),SEEK_CUR);
            fwrite(&v,sizeof(v),1,fp);

            printf("\nVictim updated successfully!\n");
            break;
        }
    }

    if(!found)
        printf("\nVictim not found!\n");

    fclose(fp);
}

void deleteVictim()
{
    printf("\n--- Delete Victim ---\n");
    FILE *fp = fopen("victim.dat", "rb");
    FILE *temp = fopen("victim_temp.dat", "wb");

    if(fp == NULL || temp == NULL)
    {
        if(fp) fclose(fp);
        if(temp) fclose(temp);
        printf("\nFile error!\n");
        return;
    }

    struct Victim v;
    char id[10];
    int found = 0;

    printf("Enter Victim ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&v, sizeof(v), 1, fp))
    {
        if(strcmp(v.id, id) != 0)
        {
            fwrite(&v, sizeof(v), 1, temp);
        }

        else
        {
            found = 1;

            FILE *sf = fopen("shelter.dat", "rb+");

            if(sf != NULL)
            {
                struct Shelter s;

                while(fread(&s, sizeof(s), 1, sf))
                {
                    if(strcmp(s.id, v.shelterId) == 0)
                    {
                        if(s.current > 0)
                            s.current--;

                        fseek(sf, -(long)sizeof(s), SEEK_CUR);
                        fwrite(&s, sizeof(s), 1, sf);

                        break;
                    }
                }

                fclose(sf);
            }
        }
    }

    fclose(fp);
    fclose(temp);

    remove("victim.dat");
    rename("victim_temp.dat", "victim.dat");

    if(found)
        printf("\nDeleted successfully!\n");
    else
        printf("\nVictim not found!\n");
}

void sortVictims()
{
    FILE *fp = fopen("victim.dat", "rb");
    if(fp == NULL)
    {
        printf("\nNo data found!\n");
        return;
    }

    struct Victim *v = NULL;
    int n = 0;

    struct Victim temp;

    while(fread(&temp, sizeof(temp), 1, fp))
    {
        struct Victim *newPtr =
            realloc(v, (n + 1) * sizeof(struct Victim));

        if(newPtr == NULL)
        {
            printf("\nMemory allocation failed!\n");
            free(v);
            fclose(fp);
            return;
        }

        v = newPtr;
        v[n++] = temp;
    }

    fclose(fp);

    for(int i = 0; i < n - 1; i++)
    {
        for(int j = 0; j < n - i - 1; j++)
        {
            if(v[j].injury < v[j+1].injury)
            {
                struct Victim temp = v[j];
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
    }

    printf("\n--- Sorted Victims (Priority) ---\n");
    for(int i = 0; i < n; i++)
    {
        printf("%s %s Injury:%d\n", v[i].id, v[i].name, v[i].injury);
    }

    free(v);
}

/* ---------- SHELTER MODULE ---------- */

void shelterMenu()
{
    int ch;

    while(1)
    {
        printf("\n--- Shelter Menu ---\n");
        printf("1. Add Shelter\n");
        printf("2. Display Shelters\n");
        printf("3. Search Shelter\n");
        printf("4. Update Shelter\n");
        printf("5. Delete Shelter\n");
        printf("6. Back\n");
        printf("7. Exit Program\n");

        printf("Enter choice: ");
        if(scanf("%d", &ch) != 1)
        {
            printf("\nInvalid input!\n");
            while(getchar()!='\n');
            continue;
        }

        switch(ch)
        {
            case 1: addShelter(); break;
            case 2: displayShelters(); break;
            case 3: searchShelter(); break;
            case 4: updateShelter(); break;
            case 5: deleteShelter(); break;
            case 6: return;
            case 7:
                printf("\nProgram Closed\n");
                exit(0);
            default: printf("\nInvalid choice!\n");
        }
    }
}

void addShelter()
{
    printf("\n--- Add Shelter ---\n");
    FILE *fp = fopen("shelter.dat", "ab");
    if(fp == NULL)
    {
        printf("\nFile error!\n");
        return;
    }

    struct Shelter s;

    printf("Enter Shelter ID: ");
    scanf("%s", s.id);
    normalizeID(s.id);

    FILE *check = fopen("shelter.dat", "rb");
    struct Shelter temp;

    if(check != NULL)
    {
        while(fread(&temp, sizeof(temp), 1, check))
        {
            if(strcmp(temp.id, s.id) == 0)
            {
                printf("\nID already exists!\n");
                fclose(check);
                fclose(fp);
                return;
            }
        }
        fclose(check);
    }

    clearBuffer();
    printf("Enter Shelter Name: ");
    fgets(s.name, sizeof(s.name), stdin);
    s.name[strcspn(s.name, "\n")] = 0;

    printf("Enter Location: ");
    fgets(s.location, sizeof(s.location), stdin);
    s.location[strcspn(s.location, "\n")] = 0;
    normalizeID(s.location);

    printf("Enter Capacity: ");
    if(scanf("%d",&s.capacity) != 1 || s.capacity <= 0)
    {
        printf("\nInvalid Capacity\n");
        while(getchar()!='\n');
        fclose(fp);
        return;
    }

    s.current = 0;

    if(fwrite(&s, sizeof(s), 1, fp) == 1)
        printf("\nShelter added successfully!\n");
    else
        printf("\nWrite failed!\n");

    fclose(fp);
}

void displayShelters()
{
    printf("\n--- Display Shelters ---\n");
    FILE *fp = fopen("shelter.dat", "rb");
    if(fp == NULL)
    {
        printf("\nNo shelter data found.\n");
        return;
    }

    struct Shelter s;

    printf("\n--- Shelter List ---\n");

    while(fread(&s, sizeof(s), 1, fp))
    {
        printf("\nID: %s\n", s.id);
        printf("Name: %s\n", s.name);
        printf("Location: %s\n", s.location);
        printf("Capacity: %d\n", s.capacity);
        printf("Current: %d\n", s.current);
        printf("Available: %d\n", s.capacity - s.current);
        printf("Status: %s\n", (s.current < s.capacity) ? "Available" : "Full");
    }

    fclose(fp);
}

void searchShelter()
{
    printf("\n--- Search Shelter ---\n");
    FILE *fp = fopen("shelter.dat", "rb");
    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Shelter s;
    char id[10];
    int found = 0;

    printf("Enter Shelter ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&s, sizeof(s), 1, fp))
    {
        if(strcmp(s.id, id) == 0)
        {
            printf("\nFound:\n");
            printf("Name: %s\n", s.name);
            printf("Location: %s\n", s.location);
            printf("Capacity: %d\n", s.capacity);
            printf("Current: %d\n", s.current);
            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nNot found!\n");

    fclose(fp);
}

void updateShelter()
{
    printf("\n--- Update Shelter ---\n");
    FILE *fp = fopen("shelter.dat", "rb+");
    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Shelter s;
    char id[10];
    int found = 0;

    printf("Enter Shelter ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&s, sizeof(s), 1, fp))
    {
        if(strcmp(s.id, id) == 0)
        {
            int newCap;
            printf("Enter new capacity: ");
            if(scanf("%d",&newCap) != 1 || newCap <= 0)
            {
                printf("\nInvalid Capacity\n");
                while(getchar()!='\n');
                fclose(fp);
                return;
            }

            if(newCap < s.current)
            {
                printf("\nCannot reduce below current occupants!\n");
            }
            else
            {
                s.capacity = newCap;
                fseek(fp, -(long)sizeof(s), SEEK_CUR);
                fwrite(&s, sizeof(s), 1, fp);
                printf("\nUpdated successfully!\n");
            }

            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nShelter not found!\n");

    fclose(fp);
}

void deleteShelter()
{
    printf("\n--- Delete Shelter ---\n");
    FILE *fp = fopen("shelter.dat", "rb");
    FILE *temp = fopen("shelter_temp.dat", "wb");

    if(fp == NULL || temp == NULL)
    {
        if(fp) fclose(fp);
        if(temp) fclose(temp);
        printf("\nFile error!\n");
        return;
    }

    struct Shelter s;
    char id[10];
    int found = 0;

    printf("Enter Shelter ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&s, sizeof(s), 1, fp))
    {
        if(strcmp(s.id, id) != 0)
        {
            fwrite(&s, sizeof(s), 1, temp);
        }
        else
        {
            if(s.current > 0)
            {
                printf("\nCannot delete occupied shelter!\n");

                fclose(fp);
                fclose(temp);

                remove("shelter_temp.dat");
                return;
            }
            found = 1;
        }
    }

    fclose(fp);
    fclose(temp);

    remove("shelter.dat");
    rename("shelter_temp.dat", "shelter.dat");

    if(found)
        printf("\nDeleted successfully!\n");
    else
        printf("\nShelter not found!\n");
}

/* ---------- RESOURCE MODULE ---------- */

void resourceMenu()
{
    int ch;

    while(1)
    {
        printf("\n--- Resource Menu ---\n");
        printf("1. Add Resource\n");
        printf("2. Display Resources\n");
        printf("3. Search Resource\n");
        printf("4. Update Resource\n");
        printf("5. Delete Resource\n");
        printf("6. Back\n");
        printf("7. Exit Program\n");

        printf("Enter choice: ");
        if(scanf("%d",&ch) != 1)
        {
            printf("\nInvalid input!\n");
            while(getchar()!='\n');
            continue;
        }

        switch(ch)
        {
            case 1: addResource(); break;
            case 2: displayResources(); break;
            case 3: searchResource(); break;
            case 4: updateResource(); break;
            case 5: deleteResource(); break;
            case 6: return;
            case 7:
                printf("\nProgram Closed\n");
                exit(0);
            default: printf("\nInvalid choice!\n");
        }
    }
}

void addResource()
{
    printf("\n--- Add Resource ---\n");
    FILE *fp = fopen("resources.dat", "ab");
    if(fp == NULL)
    {
        printf("\nFile error!\n");
        return;
    }

    struct Resource r;

    printf("Enter Resource ID: ");
    scanf("%s", r.id);
    normalizeID(r.id);

    FILE *check = fopen("resources.dat", "rb");
    struct Resource temp;

    if(check != NULL)
    {
        while(fread(&temp, sizeof(temp), 1, check))
        {
            if(strcmp(temp.id, r.id) == 0)
            {
                printf("\nID already exists!\n");
                fclose(check);
                fclose(fp);
                return;
            }
        }
        fclose(check);
    }

    clearBuffer();
    printf("Enter Resource Name: ");
    fgets(r.name, sizeof(r.name), stdin);
    r.name[strcspn(r.name, "\n")] = 0;

    printf("Enter Type (Food/Water/Medicine/etc.): ");
    fgets(r.type, sizeof(r.type), stdin);
    r.type[strcspn(r.type, "\n")] = 0;

    printf("Enter Quantity: ");
    if(scanf("%d",&r.quantity) != 1 || r.quantity <= 0)
    {
        printf("\nInvalid Quantity\n");
        while(getchar()!='\n');
        fclose(fp);
        return;
    }

    if(fwrite(&r, sizeof(r), 1, fp) == 1)
        printf("\nResource added successfully!\n");
    else
        printf("\nWrite failed!\n");

    fclose(fp);

}

void displayResources()
{
    printf("\n--- Display Resources ---\n");
    FILE *fp = fopen("resources.dat", "rb");
    if(fp == NULL)
    {
        printf("\nNo resource data found.\n");
        return;
    }

    struct Resource r;

    printf("\n--- Resource List ---\n");

    while(fread(&r, sizeof(r), 1, fp))
    {
        printf("\nID: %s\n", r.id);
        printf("Name: %s\n", r.name);
        printf("Type: %s\n", r.type);
        printf("Quantity: %d\n", r.quantity);
    }

    fclose(fp);
}

void searchResource()
{
    printf("\n--- Search Resource ---\n");
    FILE *fp = fopen("resources.dat", "rb");
    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Resource r;
    char id[10];
    int found = 0;

    printf("Enter Resource ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&r, sizeof(r), 1, fp))
    {
        if(strcmp(r.id, id) == 0)
        {
            printf("\nFound Resource:\n");
            printf("Name: %s\n", r.name);
            printf("Type: %s\n", r.type);
            printf("Quantity: %d\n", r.quantity);
            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nResource not found!\n");

    fclose(fp);
}

void updateResource()
{
    printf("\n--- Update Resource ---\n");
    FILE *fp = fopen("resources.dat", "rb+");
    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Resource r;
    char id[10];
    int found = 0;

    printf("Enter Resource ID to update: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&r, sizeof(r), 1, fp))
    {
        if(strcmp(r.id, id) == 0)
        {
            printf("Enter new Quantity: ");
            int qty;
            if(scanf("%d", &qty) != 1 || qty < 0)
            {
                printf("\nInvalid quantity!\n");
                while(getchar()!='\n');
                fclose(fp);
                return;
            }

            r.quantity = qty;

            fseek(fp, -(long)sizeof(r), SEEK_CUR);
            fwrite(&r, sizeof(r), 1, fp);

            printf("\nResource updated successfully!\n");
            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nResource not found!\n");

    fclose(fp);
}

void deleteResource()
{
    printf("\n--- Delete Resource ---\n");
    FILE *fp = fopen("resources.dat", "rb");
    FILE *temp = fopen("resource_temp.dat", "wb");

    if(fp == NULL || temp == NULL)
    {
        if(fp) fclose(fp);
        if(temp) fclose(temp);
        printf("\nFile error!\n");
        return;
    }

    struct Resource r;
    char id[10];
    int found = 0;

    printf("Enter Resource ID to delete: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&r, sizeof(r), 1, fp))
    {
        if(strcmp(r.id, id) != 0)
        {
            fwrite(&r, sizeof(r), 1, temp);
        }
        else
        {
            found = 1;
        }
    }

    fclose(fp);
    fclose(temp);

    remove("resources.dat");
    rename("resource_temp.dat", "resources.dat");

    if(found)
        printf("\nResource deleted successfully!\n");
    else
        printf("\nResource not found!\n");
}

/* ---------- VOLUNTEER MODULE ---------- */

void volunteerMenu()
{
    int ch;

    while(1)
    {
        printf("\n--- Volunteer Menu ---\n");
        printf("1. Add Volunteer\n");
        printf("2. Display Volunteers\n");
        printf("3. Search Volunteer\n");
        printf("4. Update Volunteer\n");
        printf("5. Delete Volunteer\n");
        printf("6. Back\n");
        printf("7. Exit Program\n");

        printf("Enter choice: ");
        if(scanf("%d", &ch) != 1)
        {
            printf("\nInvalid input!\n");
            while(getchar()!='\n');
            continue;
        }

        switch(ch)
        {
            case 1: addVolunteer(); break;
            case 2: displayVolunteers(); break;
            case 3: searchVolunteer(); break;
            case 4: updateVolunteer(); break;
            case 5: deleteVolunteer(); break;
            case 6: return;
            case 7:
                printf("\nProgram Closed\n");
                exit(0);
            default: printf("\nInvalid choice!\n");
        }
    }
}

void addVolunteer()
{
    printf("\n--- Add Volunteer ---\n");
    FILE *fp = fopen("volunteers.dat", "ab");
    if(fp == NULL)
    {
        printf("\nFile error!\n");
        return;
    }

    struct Volunteer v;

    printf("Enter Volunteer ID: ");
    scanf("%s", v.id);
    normalizeID(v.id);

    FILE *check = fopen("volunteers.dat", "rb");
    struct Volunteer temp;

    if(check != NULL)
    {
        while(fread(&temp, sizeof(temp), 1, check))
        {
            if(strcmp(temp.id, v.id) == 0)
            {
                printf("\nID already exists!\n");
                fclose(check);
                fclose(fp);
                return;
            }
        }
        fclose(check);
    }

    clearBuffer();
    printf("Enter Name: ");
    fgets(v.name, sizeof(v.name), stdin);
    v.name[strcspn(v.name, "\n")] = 0;

    printf("Enter Role (Medical/Food/Transport): ");
    fgets(v.role, sizeof(v.role), stdin);
    v.role[strcspn(v.role, "\n")] = 0;

    if(strlen(v.role) == 0)
    {
        printf("\nInvalid role!\n");
        fclose(fp);
        return;
    }

    if(!isValidRole(v.role))
    {
        printf("\nRole must be: Medical / Food / Transport\n");
        fclose(fp);
        return;
    }

    if(fwrite(&v, sizeof(v), 1, fp) == 1)
        printf("\nVolunteer added successfully!\n");
    else
        printf("\nWrite failed!\n");

    fclose(fp);
}

void displayVolunteers()
{
    printf("\n--- Display Volunteers ---\n");
    FILE *fp = fopen("volunteers.dat", "rb");
    if(fp == NULL)
    {
        printf("\nNo volunteer data found.\n");
        return;
    }

    struct Volunteer v;

    printf("\n--- Volunteer List ---\n");

    while(fread(&v, sizeof(v), 1, fp))
    {
        printf("\nID: %s\n", v.id);
        printf("Name: %s\n", v.name);
        printf("Role: %s\n", v.role);
    }

    fclose(fp);
}

void searchVolunteer()
{
    printf("\n--- Search Volunteer ---\n");
    FILE *fp = fopen("volunteers.dat", "rb");
    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Volunteer v;
    char id[10];
    int found = 0;

    printf("Enter Volunteer ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&v, sizeof(v), 1, fp))
    {
        if(strcmp(v.id, id) == 0) 
        {
            printf("\nFound:\n");
            printf("Name: %s\n", v.name);
            printf("Role: %s\n", v.role);
            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nVolunteer not found!\n");

    fclose(fp);
}

void updateVolunteer()
{
    printf("\n--- Update Volunteer ---\n");
    FILE *fp = fopen("volunteers.dat", "rb+");
    if(fp == NULL)
    {
        printf("\nFile not found!\n");
        return;
    }

    struct Volunteer v;
    char id[10];
    int found = 0;

    printf("Enter Volunteer ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&v, sizeof(v), 1, fp))
    {
        if(strcmp(v.id, id) == 0) 
        {
            printf("Enter new role: ");
            clearBuffer();
            fgets(v.role, sizeof(v.role), stdin);
            v.role[strcspn(v.role, "\n")] = 0;

            if(!isValidRole(v.role))
            {
                printf("\nRole must be: Medical / Food / Transport\n");
                fclose(fp);
                return;
            }

            fseek(fp, -(long)sizeof(v), SEEK_CUR);
            fwrite(&v, sizeof(v), 1, fp);

            printf("\nUpdated successfully!\n");
            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nVolunteer not found!\n");

    fclose(fp);
}

void deleteVolunteer()
{
    printf("\n--- Delete Volunteer ---\n");
    FILE *fp = fopen("volunteers.dat", "rb");
    FILE *temp = fopen("volunteer_temp.dat", "wb");

    if(fp == NULL || temp == NULL)
    {
        if(fp) fclose(fp);
        if(temp) fclose(temp);
        printf("\nFile error!\n");
        return;
    }

    struct Volunteer v;
    char id[10];
    int found = 0;

    printf("Enter Volunteer ID: ");
    scanf("%s", id);
    normalizeID(id);

    while(fread(&v, sizeof(v), 1, fp))
    {
        if(strcmp(v.id, id) != 0)
        {
            fwrite(&v, sizeof(v), 1, temp);
        }
        else
        {
            found = 1;
        }
    }

    fclose(fp);
    fclose(temp);

    remove("volunteers.dat");
    rename("volunteer_temp.dat", "volunteers.dat");

    if(found)
        printf("\nDeleted successfully!\n");
    else
        printf("\nVolunteer not found!\n");
}

/* ---------- DISTRIBUTION MODULE ---------- */

void distributionMenu()
{
    int ch;

    while(1)
    {
        printf("\n--- Distribution Menu ---\n");
        printf("1. Assign Resource to Victim\n");
        printf("2. View Distribution Records\n");
        printf("3. Back\n");
        printf("4. Exit Program\n");

        printf("Enter choice: ");
        if(scanf("%d", &ch) != 1)
        {
            printf("\nInvalid input!\n");
            while(getchar()!='\n');
            continue;
        }

        switch(ch)
        {
            case 1: assignResource(); break;
            case 2: displayDistributions(); break;
            case 3: return;
            case 4:
                printf("\nProgram Closed\n");
                exit(0);
            default: printf("\nInvalid choice!\n");
        }
    }
}

void assignResource()
{
    printf("\n--- Assign Resource ---\n");
    FILE *rf = fopen("resources.dat", "rb+");
    FILE *df = fopen("distribution.dat", "ab");

    if(rf == NULL || df == NULL)
    {
        printf("\nFile error!\n");
        if(rf) fclose(rf);
        if(df) fclose(df);
        return;
    }

    struct Distribution d;
    struct Resource r;

    printf("Enter Victim ID: ");
    scanf("%s", d.victimId);
    normalizeID(d.victimId);

    printf("Enter Resource ID: ");
    scanf("%s", d.resourceId);
    normalizeID(d.resourceId);

    printf("Enter Quantity: ");

    if(scanf("%d", &d.quantity) != 1 || d.quantity <= 0)
    {
        printf("\nInvalid quantity!\n");
        fclose(rf);
        fclose(df);
        while(getchar()!='\n');
        return;
    }

    FILE *vf = fopen("victim.dat", "rb");
    int victimFound = 0;
    struct Victim v;

    if(vf != NULL)
    {
        while(fread(&v, sizeof(v), 1, vf))
        {
            if(strcmp(v.id, d.victimId) == 0)
            {
                victimFound = 1;
                break;
            }
        }
    }

    if(vf) fclose(vf);

    if(!victimFound)
    {
        printf("\nVictim not found!\n");
        fclose(rf);
        fclose(df);
        return;
    }

    int found = 0;

    while(fread(&r, sizeof(r), 1, rf))
    {
        if(strcmp(r.id, d.resourceId) == 0)
        {
            if(r.quantity >= d.quantity)
            {
                r.quantity -= d.quantity;

                /* Get exact position of this record */
                long pos = ftell(rf) - sizeof(r);
                fseek(rf, pos, SEEK_SET);

                /* Update resource safely */
                if(fwrite(&r, sizeof(r), 1, rf) == 1)
                {
                    /* Write distribution record only if update succeeds */
                    if(fwrite(&d, sizeof(d), 1, df) == 1)
                    {
                        printf("\nResource assigned successfully!\n");
                    }
                    else
                    {
                        printf("\nDistribution record failed!\n");
                    }
                }
                else
                {
                    printf("\nResource update failed!\n");
                }
            }
            else
            {
                printf("\nNot enough resources available!\n");
            }

            found = 1;
            break;
        }
    }

    if(!found)
        printf("\nResource not found!\n");

    fclose(rf);
    fclose(df);
}

void displayDistributions()
{
    FILE *fp = fopen("distribution.dat", "rb");
    if(fp == NULL)
    {
        printf("\nNo distribution records found.\n");
        return;
    }

    struct Distribution d;

    printf("\n--- Distribution Records ---\n");

    while(fread(&d, sizeof(d), 1, fp))
    {
        printf("\nVictim ID: %s\n", d.victimId);
        printf("Resource ID: %s\n", d.resourceId);
        printf("Quantity: %d\n", d.quantity);
    }

    fclose(fp);
}

void systemSummary()
{
    FILE *fp;

    int victims = 0;
    int shelters = 0;
    int resources = 0;
    int volunteers = 0;
    int totalCapacity = 0;
    int occupied = 0;
    int fullShelters = 0;
    int maxInjury = -1;

    struct Victim v;
    struct Shelter s;
    struct Resource r;
    struct Volunteer vol;

    /* Count Victims */

    fp = fopen("victim.dat","rb");

    if(fp != NULL)
    {
        while(fread(&v,sizeof(v),1,fp))
        {
            victims++;

            if(v.injury > maxInjury)
                maxInjury = v.injury;
        }

        fclose(fp);
    }

    /* Count Shelters */

    fp = fopen("shelter.dat","rb");

    if(fp != NULL)
    {
        while(fread(&s,sizeof(s),1,fp))
        {
            shelters++;

            totalCapacity += s.capacity;
            occupied += s.current;

            if(s.current == s.capacity)
                fullShelters++;
        }

        fclose(fp);
    }

    /* Count Resources */

    fp = fopen("resources.dat","rb");

    if(fp != NULL)
    {
        while(fread(&r,sizeof(r),1,fp))
        {
            resources += r.quantity;
        }

        fclose(fp);
    }

    /* Count Volunteers */

    fp = fopen("volunteers.dat","rb");

    if(fp != NULL)
    {
        while(fread(&vol,sizeof(vol),1,fp))
            volunteers++;

        fclose(fp);
    }


    printf("\n--- System Dashboard ---\n");
    printf("Total Victims: %d\n", victims);
    printf("Total Volunteers: %d\n", volunteers);
    printf("Total Shelters: %d\n", shelters);
    printf("Full Shelters: %d\n", fullShelters);
    printf("Shelter Capacity: %d\n", totalCapacity);
    printf("Occupied Spots: %d\n", occupied);
    printf("Available Spots: %d\n", totalCapacity - occupied);
    printf("Total Resource Units: %d\n", resources);

    /* Highest injury victims */

    if(maxInjury != -1)
    {
        printf("Highest Injury Victims (Level %d): ", maxInjury);

        fp = fopen("victim.dat","rb");

        if(fp != NULL)
        {
            while(fread(&v, sizeof(v), 1, fp))
            {
                if(v.injury == maxInjury)
                {
                    printf("%s - %s\n", v.id, v.name);
                }
            }

            fclose(fp);
        }
    }
}

int isValidRole(char role[])
{
    if(strcmp(role, "Medical") == 0) return 1;
    if(strcmp(role, "Food") == 0) return 1;
    if(strcmp(role, "Transport") == 0) return 1;

    return 0;
}

void normalizeID(char id[])
{
    int i;

    while(id[0] == ' ')
        memmove(id, id + 1, strlen(id));

    int len = strlen(id);
    while(len > 0 && id[len - 1] == ' ')
    {
        id[len - 1] = '\0';
        len--;
    }

    for(i = 0; id[i]; i++)
    {
        if(id[i] >= 'a' && id[i] <= 'z')
            id[i] = id[i] - 32;
    }
}

void clearBuffer()
{
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}