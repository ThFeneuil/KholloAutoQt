#ifndef STOREDDATA_H
#define STOREDDATA_H


class StoredData
{
public:
    StoredData();

    //Getters
    int getId() const;

    //Setters
    void setId(int id);

private:
    int m_id;
};

#endif // STOREDDATA_H
