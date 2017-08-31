/* Name: Jeffrey Xu
 * Email: jeffreyhxu@gmail.com
 * ID: 404768745
 */
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <map>
using namespace std;

int main(int argc, char *argv[]){
  if(argc != 2){
    cerr << "Usage: lab3b filename" << endl;
    exit(1);
  }
  ifstream file;
  file.open(argv[1]);
  if(!file){
    cerr << "Error: Unable to open " << argv[1] << endl;
    exit(1);
  }

  bool super_found = false, group_found = false;
  int b_count, i_count, b_size, i_size, b_group, i_group, inode_1;
  int inodes_end; // the last reserved block, contiguous because there's only one group
  set<int> free_blocks;
  map<int, string> used_blocks;
  set<int> free_inodes;
  set<int> used_inodes;
  map<int, int> i_ref_count;
  map<int, int> d_ref_count;
  map<int, pair<int, string> > dirents; // the pair is parent dir and name
  map<int, int> parent;
  parent[2] = 2;
  ostringstream oss;
  
  string line;
  while(getline(file, line)){
    istringstream iss(line);
    if(!super_found){
      if(!line.compare(0, 11, "SUPERBLOCK,")){
	iss.ignore(100, ','); // SUPERBLOCK
	iss >> b_count;
	iss.ignore(1, ',');
	iss >> i_count;
	iss.ignore(1, ',');
	iss >> b_size;
	iss.ignore(1, ',');
	iss >> i_size;
	iss.ignore(1, ',');
	iss >> b_group;
	iss.ignore(1, ',');
	iss >> i_group;
	iss.ignore(1, ',');
	iss >> inode_1;
	super_found = true;
	file.seekg(0);
	if(i_count > i_group){
	  cerr << "You lied! There's more than one group! The spec said there'd only be one group." << endl;
	  exit(1);
	}
      }
    }
    else if(!group_found){
      if(!line.compare(0, 6, "GROUP,")){
	int iblock;
	iss.ignore(100, ','); // GROUP
	iss.ignore(100, ','); // group number (0, since there's only 1)
	iss.ignore(100, ','); // block count (unnecessary since only 1 group, can use superblock's count)
	iss.ignore(100, ','); // inode count (with only 1 group, equal to superblock's count)
	iss.ignore(100, ','); // free blocks (check bitmap for this)
	iss.ignore(100, ','); // free inodes (check bitmap for this)
	iss.ignore(100, ','); // free block bitmap
	iss.ignore(100, ','); // free inode bitmap
	iss >> iblock;
	inodes_end = iblock + (i_count * i_size - 1) / b_size;
	group_found = true;
	file.seekg(0);
      }
    }
    else{
      if(!line.compare(0, 6, "BFREE,")){
	int fb;
	iss.ignore(100, ',');
	iss >> fb;
	free_blocks.insert(fb);
      }
      else if(!line.compare(0, 6, "IFREE,")){
	int fi;
	iss.ignore(100, ',');
	iss >> fi;
	free_inodes.insert(fi);
      }
      else if(!line.compare(0, 6, "INODE,")){
	int inum, lcount, fsize, ibcount;
	char ftype;
	iss.ignore(100, ','); // INODE
	iss >> inum;
	iss.ignore(1, ',');
	iss >> ftype;
	iss.ignore(1, ',');
	iss.ignore(100, ','); // mode
	iss.ignore(100, ','); // owner
	iss.ignore(100, ','); // group
	iss >> lcount;
	iss.ignore(1, ',');
	iss.ignore(100, ','); // change time
	iss.ignore(100, ','); // mod time
	iss.ignore(100, ','); // access time
	iss >> fsize;
	iss.ignore(1, ',');
	iss >> ibcount;
	iss.ignore(1, ',');
	used_inodes.insert(inum);
	i_ref_count[inum] = lcount;
	//cerr << "Inode " << inum << ": ";
	for(int b = 0; b < 12; b++){
	  int db;
	  iss >> db;
	  //cerr << db << ", ";
	  if(db == 0){}
	  else if(db < 0 || db >= b_count)
	    cout << "INVALID BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << b << endl;
	  else if(db <= inodes_end)
	    cout << "RESERVED BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << b << endl;
	  else{
	    map<int, string>::iterator used = used_blocks.find(db);
	    if(used != used_blocks.end()){
	      if((*used).second.compare("Printed")){
		cout << (*used).second;
		(*used).second = "Printed";
	      }
	      cout << "DUPLICATE BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << b << endl;
	    }
	    else{
	      oss.str("");
	      oss << "DUPLICATE BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << b << endl;
	      used_blocks[db] = oss.str();
	      oss.str("");
	    }
	  }
	  if(ftype == 's')
	    break;
	  iss.ignore(1, ',');
	}
	if(ftype != 's'/* && fsize / b_size > 12*/){
	  int db;
	  iss >> db;
	  //cerr << db << ", ";
	  iss.ignore(1, ',');
	  if(db == 0){}
	  else if(db < 0 || db >= b_count)
	    cout << "INVALID INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET 12" << endl;
	  else if(db <= inodes_end)
	    cout << "RESERVED INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET 12" << endl;
	  else{
	    map<int, string>::iterator used = used_blocks.find(db);
	    if(used != used_blocks.end()){
	      if((*used).second.compare("Printed")){
		cout << (*used).second;
		(*used).second = "Printed";
	      }
	      cout << "DUPLICATE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET 12" << endl;
	    }
	    else{
	      oss.str("");
	      oss << "DUPLICATE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET 12" << endl;
	      used_blocks[db] = oss.str();
	      oss.str("");
	    }
	  }
	}
	if(ftype != 's'/* && fsize / b_size > 12 + b_size / 4*/){
	  int db;
	  iss >> db;
	  //cerr << "db, ";
	  iss.ignore(1, ',');
	  if(db == 0){}
	  else if(db < 0 || db >= b_count)
	    cout << "INVALID DOUBLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 << endl;
	  else if(db <= inodes_end)
	    cout << "RESERVED DOUBLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 << endl;
	  else{
	    map<int, string>::iterator used = used_blocks.find(db);
	    if(used != used_blocks.end()){
	      if((*used).second.compare("Printed")){
		cout << (*used).second;
		(*used).second = "Printed";
	      }
	      cout << "DUPLICATE DOUBLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 << endl;
	    }
	    else{
	      oss.str("");
	      oss << "DUPLICATE DOUBLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 << endl;
	      used_blocks[db] = oss.str();
	      oss.str("");
	    }
	  }
	}
	if(ftype != 's'/* && fsize / b_size > 12 + b_size / 4 + (b_size / 4) * (b_size / 4)*/){
	  int db;
	  iss >> db;
	  //cerr << db << ", ";
	  iss.ignore(1, ',');
	  if(db == 0){}
	  else if(db < 0 || db >= b_count)
	    cout << "INVALID TRIPPLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 + (b_size / 4) * (b_size / 4) << endl;
	  else if(db <= inodes_end)
	    cout << "RESERVED TRIPPLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 + (b_size / 4) * (b_size / 4) << endl;
	  else{
	    map<int, string>::iterator used = used_blocks.find(db);
	    if(used != used_blocks.end()){
	      if((*used).second.compare("Printed")){
		cout << (*used).second;
		(*used).second = "Printed";
	      }
	      cout << "DUPLICATE TRIPPLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 + (b_size / 4) * (b_size / 4) << endl;
	    }
	    else{
	      oss.str("");
	      oss << "DUPLICATE TRIPPLE INDIRECT BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << 12 + b_size / 4 + (b_size / 4) * (b_size / 4) << endl;
	      used_blocks[db] = oss.str();
	      oss.str("");
	    }
	  }
	}
	//cerr << endl;
      }
      else if(!line.compare(0, 7, "DIRENT,")){
	int diri, refi;
	string refname;
	iss.ignore(100, ','); // DIRENT
	iss >> diri;
	iss.ignore(1, ',');
	iss.ignore(100, ','); // byte offset
	iss >> refi;
	iss.ignore(1, ',');
	iss.ignore(100, ','); // entry length
	iss.ignore(100, ','); // name length
	iss >> refname;
	//refname = refname.substr(1, refname.length() - 2);
	d_ref_count[refi]++;
	pair<int, string> pdi;
	pdi.first = diri;
	pdi.second = refname;
	dirents[refi] = pdi;
	if(!refname.compare("'.'")){
	  if(refi != diri)
	    cout << "DIRECTORY INODE " << diri << " NAME '.' LINK TO INODE " << refi << " SHOULD BE " << diri << endl;
	}
	else if(!refname.compare("'..'")){
	  if(parent.find(diri) == parent.end())
	    parent[diri] = refi;
	  else if(parent[diri] != refi)
	    cout << "DIRECTORY INODE " << diri << " NAME '..' LINK TO INODE " << refi << " SHOULD BE " << parent[diri] << endl;
	}
	else{
	  if(parent.find(refi) == parent.end())
	    parent[refi] = diri;
	  else if(parent[refi] != diri)
	    cout << "DIRECTORY INODE " << refi << " NAME '..' LINK TO INODE " << parent[refi] << " SHOULD BE " << diri << endl;
	}
      }
      else if(!line.compare(0, 9, "INDIRECT,")){
	int inum, indnum, byteoff, bsource, db;
	iss.ignore(100, ','); // INDIRECT
	iss >> inum;
	iss.ignore(1, ',');
	iss >> indnum;
	iss.ignore(1, ',');
	iss >> byteoff;
	iss.ignore(1, ',');
	iss >> bsource;
	iss.ignore(1, ',');
	iss >> db;
	string inds;
	switch(indnum){
	case 1:
	  inds = "";
	  break;
	case 2:
	  inds = "INDIRECT ";
	  break;
	case 3:
	  inds = "DOUBLE INDIRECT ";
	  break;
	}
	int bloff = byteoff / b_size;
	if(db < 0 || db >= b_count)
	  cout << "INVALID " << inds << "BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << bloff  << endl;
	else if(db <= inodes_end)
	  cout << "RESERVED " << inds << "BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << bloff << endl;
	else{
	  map<int, string>::iterator used = used_blocks.find(db);
	  if(used != used_blocks.end()){
	    if((*used).second.compare("Printed")){
	      cout << (*used).second;
	      (*used).second = "Printed";
	    }
	    cout << "DUPLICATE " << inds << "BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << bloff << endl;
	  }
	  else{
	    oss.str("");
	    oss << "DUPLICATE " << inds << "BLOCK " << db << " IN INODE " << inum << " AT OFFSET " << bloff << endl;
	    used_blocks[db] = oss.str();
	    oss.str("");
	  }
	}
      }      
    }
  }

  for(int block = inodes_end + 1; block < b_count; block++){
    bool free = (free_blocks.find(block) != free_blocks.end());
    bool used = (used_blocks.find(block) != used_blocks.end());
    if(!free && !used)
      cout << "UNREFERENCED BLOCK " << block << endl;
    if(free && used)
      cout << "ALLOCATED BLOCK " << block << " ON FREELIST" << endl;
  }
  bool free2 = (free_inodes.find(2) != free_inodes.end());
  bool used2 = (used_inodes.find(2) != used_inodes.end());
  if(!free2 && !used2)
    cout << "UNALLOCATED INODE 2 NOT ON FREELIST" << endl;
  if(free2 && used2)
    cout << "ALLOCATED INODE 2 ON FREELIST" << endl;
  for(int inode = inode_1; inode <= i_count; inode++){
    bool free = (free_inodes.find(inode) != free_inodes.end());
    bool used = (used_inodes.find(inode) != used_inodes.end());
    if(!free && !used)
      cout << "UNALLOCATED INODE " << inode << " NOT ON FREELIST" << endl;
    if(free && used)
      cout << "ALLOCATED INODE " << inode << " ON FREELIST" << endl;
  }
  for(map<int, pair<int, string> >::iterator di = dirents.begin(); di != dirents.end(); di++){
    if(((*di).first < inode_1 && (*di).first != 2) || (*di).first > i_count)
      cout << "DIRECTORY INODE " << (*di).second.first << " NAME " << (*di).second.second << " INVALID INODE " << (*di).first << endl;
    else if(used_inodes.find((*di).first) == used_inodes.end())
      cout << "DIRECTORY INODE " << (*di).second.first << " NAME " << (*di).second.second << " UNALLOCATED INODE " << (*di).first << endl;
  }
  for(map<int, int>::iterator ri = i_ref_count.begin(); ri != i_ref_count.end(); ri++){
    if((*ri).second != d_ref_count[(*ri).first])
      cout << "INODE " << (*ri).first << " HAS " << d_ref_count[(*ri).first] << " LINKS BUT LINKCOUNT IS " << (*ri).second << endl;
  }
  file.close();
}

  
