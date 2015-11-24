#ifndef __BoundedBufferH__
#define __BoundedBufferH__

#include <assert.h>    // For error-checking purposes

//-------------------------------------------------
// Main structure of BoundedBuffer Class:
//-------------------------------------------------

template <typename Elem>
class BoundedBuffer
{
  public:
    BoundedBuffer(int MaxSize=30000);
    BoundedBuffer(const BoundedBuffer<Elem> &OtherBoundedBuffer);
    ~BoundedBuffer(void);

    void       Enqueue(const Elem &Item);    // Adds Item to BoundedBuffer end
    Elem       Dequeue(void);                // Returns Item from BoundedBuffer
    inline int ElemNum(void);                // Returns Number of Elements
    bool isBufferFull();
    bool isBufferEmpty();
    int       ElemCount;

  protected:
    Elem     *Data;      // The actual Data array
    const int MAX_NUM;   // The actual spaces will be one more than this
    int       Beginning, // Numbered location of the start and end
              End;

    // Instead of calculating the number of elements, using this variable
    // is much more convenient.
};

// BoundedBuffer Constructor function
template <typename Elem>
BoundedBuffer<Elem>::BoundedBuffer(int MaxSize) :
    MAX_NUM( MaxSize )   // Initialize the constant
{
  // This extra space added will allow us to distinguish between
  // the Beginning and the End locations.
  Data      = new Elem[MAX_NUM + 1];
  Beginning = 0;
  End       = 0;
  ElemCount = 0;
}

// BoundedBuffer Copy Constructor function
template <typename Elem>
BoundedBuffer<Elem>::BoundedBuffer(const BoundedBuffer &OtherBoundedBuffer) : MAX_NUM( OtherBoundedBuffer.MAX_NUM )  // Initialize the constant
{
  Beginning = OtherBoundedBuffer.Beginning;
  End       = OtherBoundedBuffer.End;
  ElemCount = OtherBoundedBuffer.ElemCount;

  Data      = new Elem[MAX_NUM + 1];
  for (int i = 0; i < MAX_NUM; i++)
    Data[i] = OtherBoundedBuffer.Data[i];
}

// BoundedBuffer Destructor function
template <typename Elem>
BoundedBuffer<Elem>::~BoundedBuffer(void)
{
  delete[] Data;
}

// Enqueue() function
template <typename Elem>
void BoundedBuffer<Elem>::Enqueue(const Elem &Item)
{
  // Error Check: Make sure we aren't exceeding our maximum storage space
  assert( ElemCount < MAX_NUM );

  Data[ End++ ] = Item;
  ++ElemCount;

  // Check for wrap-around
  if (End > MAX_NUM)
    End -= (MAX_NUM + 1);
}

// Dequeue() function
template <typename Elem>
Elem BoundedBuffer<Elem>::Dequeue(void)
{
  // Error Check: Make sure we aren't dequeueing from an empty queue
  assert( ElemCount > 0 );

  Elem ReturnValue = Data[ Beginning++ ];
  --ElemCount;

  // Check for wrap-around
  if (Beginning > MAX_NUM)
    Beginning -= (MAX_NUM + 1);

  return ReturnValue;
}

// ElemNum() function
template <typename Elem>
inline int BoundedBuffer<Elem>::ElemNum(void)
{
  return ElemCount;
}

// isBufferFull() function
template <typename Elem>
bool BoundedBuffer<Elem>::isBufferFull(){
  return ElemCount >= MAX_NUM;
}

// isBufferEmpty() function
template <typename Elem>
bool BoundedBuffer<Elem>::isBufferEmpty(){
  return ElemCount <= 0;
}

#endif
