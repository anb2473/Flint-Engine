Instead of taking Obj id and obj format id individually, we need to take them both as the ObjLocation struct

Handle the .env (We somehow still havent even touched that functionality)

Gracefull handle a trailing \ in the db_path arguments

Check proper freeing so we do not leave dangeling objects

We need to memorize emptied locations inside the .obj file to be filled up in post

In mk obj sanitize content to remove \x01, \x02, and \x03

Next Entry Might not be the next location. The next id might have been removed, and now its just an empty location

Check proper Attributetype icd handling, free function unused

Rename outer and inner icd variables to make sense, and extract variables in code cleanup

Optimization in the indexing script, in the set_inner_array foo we unecissarily reload the inner array every buffer, optimize for performance, avoid memory conflict

You should be able to set up a system which, if the free list gets too long, it triggers a automatic system to resize the array to clear out any unused slots (this is an innefficient task, which is why it makes sense to only do it sparsely)

Also i have an interesting method for defining the structures of the schema. What I am currently doing is each structure, equivalent to a SQL table, in the schema, has an associated name and then the actual structure content, and then an ID. The ID is automatically assigned based on the structures location inside the schema file, so the first one is 0, the second is 1, etc. However, this brings me some concerns. mainly, I worry someone is gonna create an object, it gets added to the logs with the integer id to the table, and then they are gonna move the structure up or down in the file, maybe add a new structure in the middle of the file, and then the objects will be pointing to the wrong location. My solution to this problem was save a duplicate copy of the schema on indexing, then next index check for changes, and alter every single id to reflect what it should be with those changes

I think the best solution is to assign ids via hashing. The issue with that is a), someone can change the name of the structure, and then every single object needs to be changes. Thats why i originally chose locational indexing. We can track changes and stuff, which means you can fully change the name of a structure and it will still work, which removes risk of unexpected failure. I can also add a system which checks the number of single character changes that would be needed to change one string to another so that if you change a name and restructure the file we can still easily see what its supposed to be and just give the user a quick confirmation question so everyone gets an easy time 

And then if a schema has a full change in its actual value I want to allow the user to define a script to handle migrating from the old format to the new format, or wipe the data 

My goal is just pretty simple. Someone should be able to completely delete a structure and it shouldnt fail, someone should be able to rename a structure, move it around in the file, change its values, and it shouldnt fail. Someone should be able to mess with the schema until its unrecognizable and it should still not fail

I plan to just have a migration.flint file which contains an exact line by line copy of the schema heavily compressed. When we are not in production, it will check before indexing (wont check in production for efficiency)

Also, this system will be very forward thinking unlike SQL. SQL is rigid. This will be designed for the use cases I am facing. It has built in environment variables, so you can easily manage them. It has a relational database system. It is designed to allow you to plug in your own scripts wherever you want to customize how it handles data. I want you to be able to tag watch scripts to data which will run in the background and do certian tasks. The flint engine is a suggestion, not the rule. 