in int gl_VertexID;
in int gl_InstanceID;
out gl_PerVertex{
	layout(location=0) vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};
