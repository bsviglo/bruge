<posteffect name="ssaa">
	
	<!--
	<pass name="copy" rt="BBCopy" clearRT="false" enabled="false">
		<material name="copyBB">
		  <properties/>
		</material>
		<renderStates/>
	</pass>
	-->
	
	<pass name="test" rt="BB" clearRT="false" enabled="false">
		<material name="test">
		  <properties>
			<property name="g_blendFactor" type="float" value="0.0">
				<ui type="slider" range="vec2f(0,1)" step="0.05"/>
			</property>
			<property name="g_debug" type="float" value="1">
				<ui type="checkbox"/>
			</property>
			<property name="g_sourceTex" type="texture" value="BBCopy"/>
		  </properties>
		</material>
		<renderStates>
			<!-- <state name="depthEnabled" value="true"/> -->
		</renderStates>
	</pass>
	
	<!-- ... another posteffect's passes put here. -->	
		
</posteffect>