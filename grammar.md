funcall =
          expr
         | expr funcall

pattern =
          sym
         | "_"


lambdaargs =
             pattern
            | pattern "," lambdaargs
expr =
       int
      | sym
      | "(" expr ")
      | funcall
      | "\" lambdaargs "->" expr

