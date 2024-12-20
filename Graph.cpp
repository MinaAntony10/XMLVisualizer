#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

struct User
{
    string id;
    string name;
    vector<string> posts;
    vector<string> Followers_id;
};
// Weighted, Directed Graph
class Graph
{
private:
    int numVer;
    int maxVer;
    User *vertices;
    int **edges;
    bool *marks;

public:
    Graph(int max)
    {
        numVer = 0;
        maxVer = max;
        vertices = new User[max];
        edges = new int *[max];
        for (int i = 0; i < max; i++)
        {
            edges[i] = new int[max];
            for (int j = 0; j < max; j++)
            {
                edges[i][j] = 0;
            }
        }
        marks = new bool[max];
        for (int i = 0; i < max; i++)
            marks[i] = false;
    }

    ~Graph()
    {
        delete[] vertices;
        for (int i = 0; i < maxVer; i++)
        {
            delete[] edges[i];
        }
        delete[] edges;
        delete[] marks;
    }

    void AddVertex(User vertex)
    {
        if (numVer < maxVer)
        {
            vertices[numVer] = vertex;
            numVer++;
        }
        else
        {
            cout << "Max vertices reached, cannot add more users." << endl;
        }
    }

    void AddEdge(User fromVertex, User toVertex, int weight)
    {
        int row = indexOf(fromVertex);
        int col = indexOf(toVertex);
        if (row != -1 && col != -1)
        {
            edges[row][col] = weight;
        }
        else
        {
            cout << "Invalid user IDs for edge." << endl;
        }
    }
    void clearMarks()
    {
        for (int i = 0; i < numVer; i++)
            marks[i] = false;
    }

    int indexOf(User vertex)
    {
        for (int i = 0; i < numVer; i++)
        {
            if (vertices[i].id == vertex.id)
                return i; // Compare by ID
        }
        return -1;
    }
    int indexOf(string id)
    {
        for (int i = 0; i < numVer; i++)
        {
            if (vertices[i].id == id)
                return i; // Compare by ID
        }
        return -1;
    }

    void parseXML(string filename)
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cout << "Failed to open file.\n";
            return;
        }

        string line;
        string currentId, currentName;
        vector<string> currentPosts;
        vector<string> currentFollowers;
        bool readingPosts = false, readingFollowers = false, readingID = false; // flags to indicate start of nested tag

        while (getline(file, line))
        {
            if (line.empty())
                continue;

            if (line.find("<user>") != string::npos)
            {
                currentPosts.clear();
                currentFollowers.clear();
                readingID = true;
            }

            if (readingID && line.find("<id>") != string::npos)
            {
                currentId = line.substr(line.find("<id>") + 4, line.find("</id>") - line.find("<id>") - 4);
                readingID = false;
            }
            if (line.find("</id>") != string::npos)
            {
                readingID = false;
            }

            if (line.find("<name>") != string::npos)
            {
                currentName = line.substr(line.find("<name>") + 6, line.find("</name>") - line.find("<name>") - 6);
            }

            if (line.find("<posts>") != string::npos)
            {
                readingPosts = true;
            }

            if (readingPosts && line.find("<post>") != string::npos)
            {
                string post = line.substr(line.find("<post>") + 6, line.find("</post>") - line.find("<post>") - 6);
                currentPosts.push_back(post);
            }

            if (line.find("</posts>") != string::npos)
            {
                readingPosts = false;
            }

            if (line.find("<followers>") != string::npos)
            {
                readingFollowers = true;
            }

            if (readingFollowers && line.find("<id>") != string::npos)
            {
                string followerId = line.substr(line.find("<id>") + 4, line.find("</id>") - line.find("<id>") - 4);
                currentFollowers.push_back(followerId);
            }

            if (line.find("</followers>") != string::npos)
            {
                readingFollowers = false;
            }

            if (line.find("</user>") != string::npos)
            {
                User newUser = {currentId, currentName, currentPosts, currentFollowers};
                AddVertex(newUser);
            }
        }
        file.close();
        addEdgesBetweenUsers();
    }

    void addEdgesBetweenUsers()
    {
        for (int i = 0; i < numVer; i++)
        {
            User currentUser = vertices[i];
            for (string followerId : currentUser.Followers_id)
            {
                int followerIndex = indexOf(followerId);
                if (followerIndex != -1)
                {
                    User newFollower = vertices[followerIndex];
                    AddEdge(newFollower, currentUser, 1);
                }
                else
                {
                    // Create Unknown User if follower is not found
                    User newFollower = {followerId, "Unknown User", vector<string>(), vector<string>()};
                    AddVertex(newFollower);
                    AddEdge(newFollower, currentUser, 1);
                }
            }
        }
    }

    User most_influencer()
    {
        int maxFollowers = -1;
        User mostInfluentialUser;
        // Traverse each user and count the followers
        for (int i = 0; i < numVer; i++)
        {
            int followerCount = vertices[i].Followers_id.size();
            if (followerCount > maxFollowers)
            {
                maxFollowers = followerCount;
                mostInfluentialUser = vertices[i];
            }
        }
        return mostInfluentialUser;
    }

    vector<string> searchPosts(string searchTerm)
    {
        vector<string> matchedPosts;
        for (int i = 0; i < numVer; i++)
        {
            for (string post : vertices[i].posts)
            {
                if (post.find(searchTerm) != string::npos)
                {
                    matchedPosts.push_back("User: " + vertices[i].name + " (ID: " + vertices[i].id + ") - " + post);
                }
            }
        }
        return matchedPosts;
    }

    void display()
    {
        for (int i = 0; i < numVer; i++)
        {
            cout << "User Name: " << vertices[i].name << endl;
            cout << "User ID  : " << vertices[i].id << endl;
            cout << "Posts: \n";
            for (string post : vertices[i].posts)
            {
                cout << post << "\n";
            }
            cout << "\nFollowers: ";
            for (int j = 0; j < numVer; j++)
            {
                if (edges[j][i] != 0)
                {
                    cout << vertices[j].name << ", ";
                }
            }
            cout << "\n======================================\n";
        }
    }
};
User most_active()
{
    int maxActivity = -1;
    User mostActiveUser;

    for (int i = 0; i < numVer; i++)
    {
        int activityScore = 0;

        for (int j = 0; j < numVer; j++)
        {
            if (edges[i][j] != 0)
            {
                activityScore++;
            }
        }

        if (activityScore > maxActivity)
        {
            maxActivity = activityScore;
            mostActiveUser = vertices[i];
        }
    }

    return mostActiveUser;
}

int main()
{
    Graph Network(30);
    Network.parseXML("Network.xml");
    Network.display();
    return 0;

    User mostInfluential = Network.most_influencer();
    cout << "Most Influential User: " << mostInfluential.name << "  ID: " << mostInfluential.id << endl;

    cout << "=====================================\n"
         << endl;







    string searchTerm = "Charlie";
    vector<string> posts = Network.searchPosts(searchTerm);

    cout << "Posts containing '" << searchTerm << "':" << endl;
    for (const string &post : posts)
    {
        cout << post << endl;
    }

    return 0;
}
