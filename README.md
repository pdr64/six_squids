# six_squids
https://pdr64.github.io/six_squids/

Milestone3.ino : 
at this point, we have line following, wall avoiding, and a very rudimentary algorithm implemented. 

What happens now is as follows: 
1. We reach an intersection 
2. We update where we are based on which direction we're facing 
3. We get the values of the left, right, and front squares, regardless of wall existance 
4. If there is no wall there, we check if that square has already been visited. 
      i. if it has not, we add it to the stack of things to visit 
      ii. if it has, we add a wall to dataArray, which we will send via radio to the base station to update the GUI 
5. Next, we have to decide where to move. We pop the next square off the stack, and take the delta x and delta y values. 
      i. if we can just turn towards this square, we do so based on what direction we're facing 
      ii. if we cannot, we currently just turn around. 
      
This means we need to implement what to do if the next square to visit isn't right next to us. 
I think we should keep a history of where we've been and just backtrack to the nearest node. TODO. 
