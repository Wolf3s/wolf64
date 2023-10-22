// ID_PM.C

#include "wl_def.h"

word ChunksInFile;
word PMSpriteStart;
word PMSoundStart;

bool PMSoundInfoPagePadded = false;

word *pageLengths;

byte *PMPageData;
byte **PMPages;

word swapword(word w) {
    return ((w&0xff)<<8)|((w>>8)&0xff);
}

longword swaplongword(longword lw) {
    return ((lw&0xff)<<24) | ((lw & 0xff00)<<8) | ((lw >> 8)&0xff00) | ((lw>>24)&0xff);
}

/*
==================
=
= PM_Startup
=
==================
*/

void PM_Startup (void)
{
    int      i;
    int      padding;
    byte     *page;
    uint32_t *pageOffsets;
    uint32_t pagesize;
    int32_t  filesize,datasize;
    int      file;
    char     fname[13] = "vswap.";

    strcat (fname,extension);

    file = dfs_open(fname);

    if (!file)
        CA_CannotOpen(fname);

    filesize = dfs_size(file);

    //
    // read in header variables
    //
    dfs_read (&ChunksInFile,sizeof(ChunksInFile),1,file);
    ChunksInFile = swapword(ChunksInFile);
    dfs_read (&PMSpriteStart,sizeof(PMSpriteStart),1,file);
    PMSpriteStart = swapword(PMSpriteStart);
    dfs_read (&PMSoundStart,sizeof(PMSoundStart),1,file);
    PMSoundStart = swapword(PMSoundStart);

    //
    // read in the chunk offsets
    //
    pageOffsets = SafeMalloc((ChunksInFile + 1) * sizeof(*pageOffsets));

    dfs_read (pageOffsets,sizeof(*pageOffsets),ChunksInFile,file);
    for (int i = 0; i < ChunksInFile; i++)
        pageOffsets[i] = swaplongword(pageOffsets[i]);

    //
    // read in the chunk lengths
    //
    pageLengths = SafeMalloc(ChunksInFile * sizeof(*pageLengths));

    dfs_read (pageLengths,sizeof(*pageLengths),ChunksInFile,file);
    for (int i = 0; i < ChunksInFile; i++)
        pageLengths[i] = swapword(pageLengths[i]);

    datasize = filesize - pageOffsets[0];

    if (datasize < 0)
        Quit ("PM_Startup: The page file \"%s\" is too large!",fname);

    pageOffsets[ChunksInFile] = filesize;

    //
    // check that all chunk offsets are valid
    //
    for (i = 0; i < ChunksInFile; i++)
    {
        if (!pageOffsets[i])
            continue;           // sparse page

        if (pageOffsets[i] < pageOffsets[0] || pageOffsets[i] >= filesize)
            Quit ("PM_Startup: Illegal page offset for page %i: %u (filesize: %u)",i,pageOffsets[i],filesize);
    }

    //
    // calculate total amount of padding needed for sprites and sound info page
    //
    padding = 0;

    for (i = PMSpriteStart; i < PMSoundStart; i++)
    {
        if (!pageOffsets[i])
            continue;           // sparse page

        if (((pageOffsets[i] - pageOffsets[0]) + padding) & 1)
            padding++;
    }

    if (((pageOffsets[ChunksInFile - 1] - pageOffsets[0]) + padding) & 1)
        padding++;

    //
    // allocate enough memory to hold the whole page file
    //
    PMPageData = SafeMalloc(datasize + padding);

    //
    // [ChunksInFile + 1] pointers to page starts
    // the last pointer points one byte after the last page
    //
    PMPages = SafeMalloc((ChunksInFile + 1) * sizeof(*PMPages));

    //
    // load pages and initialize PMPages pointers
    //
    page = &PMPageData[0];

    for (i = 0; i < ChunksInFile; i++)
    {
        if ((i >= PMSpriteStart && i < PMSoundStart) || i == ChunksInFile - 1)
        {
            //
            // pad with zeros to make it 2-byte aligned
            //
            if ((page - &PMPageData[0]) & 1)
            {
                *page++ = 0;

                if (i == ChunksInFile - 1)
                    PMSoundInfoPagePadded = true;
            }
        }

        PMPages[i] = page;

        if (!pageOffsets[i])
            continue;               // sparse page

        //
        // use specified page length when next page is sparse
        // otherwise, calculate size from the offset difference between this and the next page
        //
        if (!pageOffsets[i + 1])
            pagesize = pageLengths[i];
        else
            pagesize = pageOffsets[i + 1] - pageOffsets[i];

        dfs_seek (file,pageOffsets[i],SEEK_SET);
        dfs_read (page,sizeof(*page),pagesize,file);

        if (i >= PMSpriteStart && i < PMSoundStart)
        {
            compshape_t *shape = (compshape_t *)page;
            shape->leftpix = swapword(shape->leftpix);
            shape->rightpix = swapword(shape->rightpix);
            for (int x = shape->leftpix; x <= shape->rightpix; x++)
                shape->dataofs[x - shape->leftpix] = swapword(shape->dataofs[x - shape->leftpix]);
        }
        if (i >= PMSoundStart && i < ChunksInFile - 1)
        {
            for (int i = 0; i < pagesize; i++)
                page[i] = page[i] == 0 ? 0 : (page[i] - 127);
        }

        page += pagesize;
    }

    //
    // last page points after page buffer
    //
    PMPages[ChunksInFile] = page;

    free (pageOffsets);

    dfs_close (file);
}


/*
==================
=
= PM_Shutdown
=
==================
*/

void PM_Shutdown (void)
{
    free (pageLengths);
    free (PMPages);
    free (PMPageData);

    pageLengths = NULL;
    PMPages = NULL;
    PMPageData = NULL;
}


/*
==================
=
= PM_GetPageSize
=
==================
*/

uint32_t PM_GetPageSize (int page)
{
    if (page < 0 || page >= ChunksInFile)
        Quit ("PM_GetPageSize: Invalid page request: %i",page);

    return (uint32_t)(PMPages[page + 1] - PMPages[page]);
}


/*
==================
=
= PM_GetPage
=
= Returns the address of the page
=
==================
*/

byte *PM_GetPage (int page)
{
    if (page < 0 || page >= ChunksInFile)
        Quit ("PM_GetPage: Invalid page request: %i",page);

    return PMPages[page];
}


/*
==================
=
= PM_GetPageEnd
=
= Returns the address of the last page
=
==================
*/

byte *PM_GetPageEnd (void)
{
    return PMPages[ChunksInFile];
}
