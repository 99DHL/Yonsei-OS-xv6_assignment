#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

// Display the chain of free segments.
void
freelist() {
  int i = 0;
  Header *p = freep;
  printf(1, "freep chain:\n");
  printf(1, "[%d] p = %p, p->s.size = %d, p->s.ptr = %p\n", i++, p, p->s.size, p->s.ptr);
  for(p = p->s.ptr; p != freep; p = p->s.ptr) {
    printf(1, "[%d] p = %p, p->s.size = %d, p->s.ptr = %p\n", i++, p, p->s.size, p->s.ptr);
  }
  printf(1, "\n");
}

void
free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;

  // Display the chain of free segments.
  freelist();
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  // This part is commented in this assignment!!
  // morecore function will allocate the exact size of requested amount, not 4096!!
  //if(nu < 4096)
    //nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep;
}

//////////////////////////////////// changed part //////////////////////////////////////////

Header* findbestfit(Header *prevp, uint nunits)
{
	// initialize bestfit
	Header* bestfit = 0;
	Header *p;

	for(p = prevp->s.ptr; ; p = p->s.ptr){
		if(p->s.size >= nunits){
			bestfit = p;
			break;
		}
		if(p == prevp)
			break;
	}

	if(bestfit == 0)
	{
		return 0;
	}

	// find bestfit
	for(p = prevp->s.ptr; ; p = p->s.ptr){
		if(p->s.size >= nunits){
			if( (p->s.size - nunits) < (bestfit->s.size - nunits) )
			{
				bestfit = p;
			}
		}
		if(p == prevp)
			break;
	}

	return bestfit;
}

///////////////////////////////////////////////////////////////////////////////////////////


void*
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;
  Header *bestfit;	// changed part(memory chunk whose size is closest to the request size)

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }

  ////////////////////////////// changed part ////////////////////////////////

  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
	bestfit = findbestfit(freep, nunits);			// find bestfit
    
	if(p == bestfit){		// if p is the best fit
	  //printf(2,"selected bestfit's size = %d\n\n", (bestfit->s.size));	// test code!!
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }

	if(p == freep)
		if((p = morecore(nunits)) == 0)
			return 0;
  }

  /////////////////////////////////////////////////////////////////////////////
}

